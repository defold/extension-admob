#!/usr/bin/env python3
"""
Fetch AdMob mediation adapter data for Android from the official "choose networks" page.
"""
import json
import re
from html import unescape
from pathlib import Path
from typing import Dict, List
from urllib.parse import urljoin

import requests
from bs4 import BeautifulSoup


MAIN_URL = "https://developers.google.com/admob/android/choose-networks"
FRAME_PREFIX = "/frame/admob/android/choose-networks"
RELNOTES_URL = "https://developers.google.com/admob/android/rel-notes"
APP_OPEN_URL = "https://developers.google.com/admob/android/app-open"
ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "extension-admob" / "manifests" / "android" / "build.gradle"
EXT_PROPERTIES = ROOT / "extension-admob" / "ext.properties"

# Fallback lifecycle version if parsing fails
LIFECYCLE_FALLBACK = "2.8.3"

# Map the "name" attribute used in the HTML checkboxes to the ext.properties key.
NETWORK_MAP = {
    "appLovin": "applovin",
    "chartboost": "chartboost",
    "fyber": "dt_exchange",
    "imobile": "imobile",
    "inmobi": "inmobi",
    "ironsource": "ironsource",
    "vungle": "liftoff",
    "line": "line",
    "maio": "maio",
    "facebook": "meta",
    "mintegral": "mintegral",
    "moloco": "moloco",
    "mytarget": "mytarget",
    "pangle": "pangle",
    "pubmatic": "pubmatic",
    "unity": "unity",
}

EXTRA_REPOS = {
    # Chartboost SDK is hosted on their repo, not on Maven Central
    "chartboost": ["maven { url 'https://cboost.jfrog.io/artifactory/chartboost-ads/' }"],
}

LABEL_MAP = {
    "applovin": "AppLovin Android",
    "chartboost": "Chartboost Android",
    "dt_exchange": "DT Exchange Android",
    "imobile": "i-mobile Android",
    "inmobi": "InMobi Android",
    "ironsource": "ironSource Android",
    "liftoff": "Liftoff Monetize (Vungle) Android",
    "line": "Line Ads Android",
    "maio": "maio Android",
    "meta": "Meta Android",
    "mintegral": "Mintegral Android",
    "moloco": "Moloco Android",
    "mytarget": "MyTarget Android",
    "pangle": "Pangle Android",
    "pubmatic": "PubMatic Android",
    "unity": "Unity Ads Android",
}

def collapse_blank_lines(lines: List[str]) -> List[str]:
    collapsed: List[str] = []
    prev_blank = False
    for line in lines:
        if line == "" and prev_blank:
            continue
        prev_blank = line == ""
        collapsed.append(line)
    return collapsed


def fetch(url: str) -> str:
    resp = requests.get(url)
    resp.raise_for_status()
    return resp.text


def find_frame_url(html: str) -> str:
    soup = BeautifulSoup(html, "html.parser")
    frame = soup.find("iframe", src=re.compile(re.escape(FRAME_PREFIX)))
    if not frame or not frame.get("src"):
        raise RuntimeError("Unable to locate mediation frame URL")
    return urljoin(MAIN_URL, frame["src"])


def version_key(v: str):
    return tuple(int(x) for x in v.split("."))


def read_current_android_version() -> str:
    if not OUTPUT.exists():
        return ""
    m = re.search(r"com\.google\.android\.gms:play-services-ads:([0-9.]+)", OUTPUT.read_text())
    return m.group(1) if m else ""


def read_current_lifecycle_version() -> str:
    if not OUTPUT.exists():
        return ""
    m = re.search(r"androidx\.lifecycle:lifecycle-process:([0-9.]+)", OUTPUT.read_text())
    return m.group(1) if m else ""


def fetch_latest_android_sdk(current_version: str = "") -> Dict[str, object]:
    html = fetch(RELNOTES_URL)
    soup = BeautifulSoup(html, "html.parser")
    versions = []
    for cell in soup.find_all("td", id=True):
        vid = cell.get("id", "")
        if not re.match(r"^\d+\.\d+\.\d+$", vid):
            continue
        row = cell.parent
        cells = row.find_all("td")
        if len(cells) >= 3:
            versions.append((vid, cells[2]))
    if not versions:
        raise RuntimeError("Unable to parse Android SDK version from release notes")
    versions.sort(key=lambda x: version_key(x[0]), reverse=True)
    latest_version, _ = versions[0]
    notes_filtered: List[Dict[str, object]] = []
    current_key = version_key(current_version) if current_version else None
    for version, cell in versions:
        if current_key and version_key(version) <= current_key:
            continue
        items = []
        for li in cell.find_all("li"):
            text = li.get_text(" ", strip=True)
            if text:
                items.append(text)
        if not items:
            raw = cell.get_text(" ", strip=True)
            if raw:
                items.append(raw)
        if items:
            notes_filtered.append({"version": version, "items": items})
    return {"version": latest_version, "notes": notes_filtered}


def fetch_lifecycle_version(current_version: str = "") -> str:
    try:
        html = fetch(APP_OPEN_URL)
        versions = re.findall(r"androidx\.lifecycle:lifecycle-process:([0-9.]+)", html)
        if versions:
            latest = max(versions, key=version_key)
            if not current_version or version_key(latest) > version_key(current_version):
                return latest
    except Exception:
        pass
    return current_version or LIFECYCLE_FALLBACK


def clean_repositories(raw: str) -> List[str]:
    if not raw:
        return []
    s = unescape(raw).strip()
    s = s.replace("maven {,", "maven {").replace(",}", "}")
    s = re.sub(r"\s+", " ", s)
    return [s]


def clean_dependencies(raw: str) -> List[str]:
    if not raw:
        return []
    deps: List[str] = []
    for chunk in raw.split(","):
        dep = chunk.strip().strip("'").strip('"')
        if dep:
            deps.append(dep)
    return deps


def parse_frame(html: str) -> Dict[str, Dict[str, object]]:
    soup = BeautifulSoup(html, "html.parser")
    results: Dict[str, Dict[str, object]] = {}
    found_names = set()
    for inp in soup.select("input[type='checkbox']"):
        name_attr = inp.get("name")
        if not name_attr:
            continue
        found_names.add(name_attr)
        key = NETWORK_MAP.get(name_attr)
        if not key:
            raise RuntimeError(f"Missing NETWORK_MAP entry for adapter name '{name_attr}'")
        version = (inp.get("value") or "").strip()
        repos = clean_repositories(inp.get("data-repositories", ""))
        dependencies = clean_dependencies(inp.get("data-dependencies", ""))
        results[key] = {
            "display_name": name_attr,
            "version": version,
            "repositories": repos,
            "dependencies": dependencies,
        }
    missing = set(NETWORK_MAP.keys()) - found_names
    if missing:
        print(f"WARNING: NETWORK_MAP contains entries not present on AdMob site (assumed removed): {sorted(missing)}\n"
              f"Update NETWORK_MAP in updater/android.py after checking https://developers.google.com/admob/android/choose-networks")
    added = found_names - set(NETWORK_MAP.keys())
    if added:
        raise RuntimeError(f"New adapters on AdMob site not in NETWORK_MAP: {sorted(added)}\n"
                           f"Please update NETWORK_MAP in updater/android.py (see https://developers.google.com/admob/android/choose-networks)")
    return results


def render_gradle(data: Dict[str, Dict[str, object]], sdk_info: Dict[str, str], lifecycle_version: str) -> str:
    play_services_version = sdk_info["version"]
    lines: List[str] = []
    lines.append("// Auto-generated by updater/android.py. Do not edit by hand.")
    lines.append("repositories {")
    lines.append("  mavenCentral()")
    lines.append("  google()")
    for key, meta in sorted(data.items()):
        repos: List[str] = meta.get("repositories", []) or []
        repos.extend(EXTRA_REPOS.get(key, []))
        for repo in repos:
            lines.append(f"  {{{{#admob.{key}_android}}}}")
            lines.append(f"  {repo}")
            lines.append(f"  {{{{/admob.{key}_android}}}}")
    lines.append("}")
    lines.append("")
    lines.append("dependencies {")
    lines.append(f"  implementation \"com.google.android.gms:play-services-ads:{play_services_version}\"")
    lines.append(f"  implementation \"androidx.lifecycle:lifecycle-common:{lifecycle_version}\"")
    lines.append(f"  implementation \"androidx.lifecycle:lifecycle-process:{lifecycle_version}\"")
    lines.append(f"  annotationProcessor \"androidx.lifecycle:lifecycle-compiler:{lifecycle_version}\"")
    lines.append("")
    for key, meta in sorted(data.items()):
        deps: List[str] = meta.get("dependencies", []) or []
        if not deps:
            continue
        lines.append(f"  {{{{#admob.{key}_android}}}}")
        for dep in deps:
            lines.append(f"  implementation '{dep}'")
        lines.append(f"  {{{{/admob.{key}_android}}}}")
        lines.append("")
    if lines[-1] == "":
        lines.pop()
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


def rewrite_ext_properties(data: Dict[str, Dict[str, object]]) -> None:
    if not EXT_PROPERTIES.exists():
        raise RuntimeError(f"ext.properties not found at {EXT_PROPERTIES}")

    managed_names = {f"{key}_android" for key in data.keys()}
    original_lines = EXT_PROPERTIES.read_text().splitlines()
    preserved: List[str] = []
    for line in original_lines:
        if any(line.startswith(f"{name}.") for name in managed_names):
            continue
        preserved.append(line)

    insert_index = None
    for idx, line in enumerate(preserved):
        if line.startswith("applovin_ios.type") or line.startswith("applovin_ios.label"):
            insert_index = idx
            break
    if insert_index is None:
        insert_index = len(preserved)

    new_block: List[str] = []
    for key in sorted(data.keys()):
        prop = f"{key}_android"
        label = LABEL_MAP.get(key, f"{key.replace('_', ' ').title()} Android")
        new_block.append(f"{prop}.type = bool")
        new_block.append(f"{prop}.label = {label}")
        new_block.append(f"{prop}.private = 1")
        new_block.append("")
    if new_block and new_block[-1] == "":
        new_block.pop()

    merged = preserved[:insert_index]
    if merged and merged[-1] != "":
        merged.append("")
    merged.extend(new_block)
    if insert_index < len(preserved):
        if merged and merged[-1] != "":
            merged.append("")
        merged.extend(preserved[insert_index:])

    merged = collapse_blank_lines(merged)
    EXT_PROPERTIES.write_text("\n".join(merged) + "\n")


def main() -> None:
    current_version = read_current_android_version()
    current_lifecycle = read_current_lifecycle_version()
    main_html = fetch(MAIN_URL)
    frame_url = find_frame_url(main_html)
    frame_html = fetch(frame_url)
    data = parse_frame(frame_html)
    sdk_info = fetch_latest_android_sdk(current_version)
    lifecycle_version = fetch_lifecycle_version(current_lifecycle)
    gradle_content = render_gradle(data, sdk_info, lifecycle_version)
    OUTPUT.write_text(gradle_content)
    rewrite_ext_properties(data)
    print(f"Wrote {OUTPUT} and updated ext.properties for {len(data)} adapters.")
    notes = sdk_info.get("notes")
    if notes:
        print(f"Android SDK version {sdk_info['version']} (release notes excerpt, see {RELNOTES_URL}):")
        for entry in notes:
            print(f"- {entry['version']}:")
            for item in entry["items"]:
                print(f"  • {item}")
    else:
        print(f"Android SDK version {sdk_info['version']} (see {RELNOTES_URL})")
    print(f"Lifecycle version for app open: {lifecycle_version} (source {APP_OPEN_URL})")


if __name__ == "__main__":
    main()
