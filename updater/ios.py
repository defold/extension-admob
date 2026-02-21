#!/usr/bin/env python3
"""
Fetch AdMob mediation adapter data for iOS from the official "choose networks" page and render the Podfile template.
"""
import re
from pathlib import Path
from typing import Dict
from urllib.parse import urljoin

import requests
from bs4 import BeautifulSoup


MAIN_URL = "https://developers.google.com/admob/ios/choose-networks"
FRAME_PREFIX = "/frame/admob/ios/choose-networks"
RELNOTES_URL = "https://developers.google.com/admob/ios/rel-notes"
QUICKSTART_URL = "https://developers.google.com/admob/ios/quick-start"
SPM_PACKAGE_URL = "https://raw.githubusercontent.com/googleads/swift-package-manager-google-mobile-ads/main/Package.swift"
ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "extension-admob" / "manifests" / "ios" / "Podfile"
EXT_PROPERTIES = ROOT / "extension-admob" / "ext.properties"

IOS_PLATFORM = "13.0"

# Map the "name" attribute from the checkbox to the ext.properties key.
NETWORK_MAP = {
    "AppLovin": "applovin",
    "Chartboost": "chartboost",
    "Fyber": "dt_exchange",
    "IMobile": "imobile",
    "InMobi": "inmobi",
    "IronSource": "ironsource",
    "Vungle": "liftoff",
    "Line": "line",
    "Maio": "maio",
    "Facebook": "meta",
    "Mintegral": "mintegral",
    "Moloco": "moloco",
    "MyTarget": "mytarget",
    "Pangle": "pangle",
    "PubMatic": "pubmatic",
    "Unity": "unity",
}

LABEL_MAP = {
    "applovin": "AppLovin iOS",
    "chartboost": "Chartboost iOS",
    "dt_exchange": "DT Exchange iOS",
    "imobile": "i-mobile iOS",
    "inmobi": "InMobi iOS",
    "ironsource": "ironSource iOS",
    "liftoff": "Liftoff Monetize (Vungle) iOS",
    "line": "Line Ads iOS",
    "maio": "maio iOS",
    "meta": "Meta iOS",
    "mintegral": "Mintegral iOS",
    "moloco": "Moloco iOS",
    "mytarget": "MyTarget iOS",
    "pangle": "Pangle iOS",
    "pubmatic": "PubMatic iOS",
    "unity": "Unity Ads iOS",
}

def collapse_blank_lines(lines: list[str]) -> list[str]:
    collapsed: list[str] = []
    prev_blank = False
    for line in lines:
        if line == "" and prev_blank:
            continue
        prev_blank = (line == "")
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


def read_current_ios_version() -> str:
    if not OUTPUT.exists():
        return ""
    m = re.search(r"pod 'Google-Mobile-Ads-SDK', '([0-9.]+)'", OUTPUT.read_text())
    return m.group(1) if m else ""


def resolve_ios_version() -> str:
    # Prefer versions from release notes table
    versions = []
    rel_html = fetch(RELNOTES_URL)
    soup = BeautifulSoup(rel_html, "html.parser")
    for row in soup.find_all("tr"):
        cells = row.find_all("td")
        if len(cells) < 3:
            continue
        ver_txt = cells[0].get_text(strip=True)
        if re.match(r"^\d+\.\d+\.\d+$", ver_txt):
            versions.append(ver_txt)
    if not versions:
        # fallback to quick start / SPM
        html = fetch(QUICKSTART_URL)
        match = re.search(r"pod\s+'Google-Mobile-Ads-SDK',\s+'([0-9.]+)'", html)
        if match:
            versions.append(match.group(1))
        try:
            pkg = fetch(SPM_PACKAGE_URL)
            versions.extend(re.findall(r"\d+\.\d+\.\d+", pkg))
        except Exception:
            pass
    if not versions:
        raise RuntimeError("Unable to determine iOS SDK version from release notes/quick start/SPM")
    return max(versions, key=version_key)


def fetch_release_notes(current_version: str, resolved_version: str) -> Dict[str, object]:
    notes_filtered: list[Dict[str, object]] = []
    notes_unavailable = False
    try:
        rel_html = fetch(RELNOTES_URL)
        soup = BeautifulSoup(rel_html, "html.parser")
        versions = []
        for row in soup.find_all("tr"):
            cells = row.find_all("td")
            if len(cells) < 3:
                continue
            ver_txt = cells[0].get_text(strip=True)
            if not re.match(r"^\d+\.\d+\.\d+$", ver_txt):
                continue
            versions.append((ver_txt, cells[2]))
        versions.sort(key=lambda x: version_key(x[0]), reverse=True)
        current_key = version_key(current_version) if current_version else None
        for vid, cell in versions:
            if current_key and version_key(vid) <= current_key:
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
                seen = set()
                unique_items = []
                for it in items:
                    if it in seen:
                        continue
                    seen.add(it)
                    unique_items.append(it)
                notes_filtered.append({"version": vid, "items": unique_items})
        if not notes_filtered:
            if versions and version_key(versions[0][0]) <= version_key(resolved_version):
                notes_unavailable = True
    except Exception:
        notes_unavailable = True
    return {"notes": notes_filtered, "notes_unavailable": notes_unavailable}


def fetch_latest_ios_sdk(current_version: str = "") -> Dict[str, object]:
    resolved_version = resolve_ios_version()
    rel_notes = fetch_release_notes(current_version, resolved_version)
    return {
        "version": resolved_version,
        "notes": rel_notes["notes"],
        "notes_unavailable": rel_notes["notes_unavailable"],
    }


def parse_frame(html: str) -> Dict[str, Dict[str, str]]:
    soup = BeautifulSoup(html, "html.parser")
    results: Dict[str, Dict[str, str]] = {}
    found_names = set()
    for inp in soup.select("input[type='checkbox']"):
        name_attr = inp.get("name")
        if not name_attr:
            continue
        found_names.add(name_attr)
        key = NETWORK_MAP.get(name_attr)
        if not key:
            raise RuntimeError(f"Missing NETWORK_MAP entry for adapter name '{name_attr}'")
        pod = f"pod 'GoogleMobileAdsMediation{name_attr}'"
        results[key] = {"display_name": name_attr, "pod": pod}
    missing = set(NETWORK_MAP.keys()) - found_names
    if missing:
        print(f"WARNING: NETWORK_MAP contains entries not present on AdMob site (assumed removed): {sorted(missing)}\n"
              f"Update NETWORK_MAP in updater/ios.py after checking https://developers.google.com/admob/ios/choose-networks")
    added = found_names - set(NETWORK_MAP.keys())
    if added:
        raise RuntimeError(f"New adapters on AdMob site not in NETWORK_MAP: {sorted(added)}\n"
                           f"Please update NETWORK_MAP in updater/ios.py (see https://developers.google.com/admob/ios/choose-networks)")
    return results


def render_podfile(data: Dict[str, Dict[str, str]], sdk_info: Dict[str, str]) -> str:
    gma_version = sdk_info["version"]
    lines = []
    lines.append("# Auto-generated by updater/ios.py. Do not edit by hand.")
    lines.append(f"platform :ios, '{IOS_PLATFORM}'")
    lines.append(f"pod 'Google-Mobile-Ads-SDK', '{gma_version}'")
    lines.append("")
    for key, meta in sorted(data.items()):
        lines.append(f"{{{{#admob.{key}_ios}}}}")
        lines.append(meta["pod"])
        lines.append(f"{{{{/admob.{key}_ios}}}}")
        lines.append("")
    if lines[-1] == "":
        lines.pop()
    lines.append("")
    return "\n".join(lines)


def rewrite_ext_properties(data: Dict[str, Dict[str, str]]) -> None:
    if not EXT_PROPERTIES.exists():
        raise RuntimeError(f"ext.properties not found at {EXT_PROPERTIES}")

    managed_names = {f"{key}_ios" for key in data.keys()}
    original_lines = EXT_PROPERTIES.read_text().splitlines()

    preserved: list[str] = []
    for line in original_lines:
        if any(line.startswith(f"{name}.") for name in managed_names):
            continue
        preserved.append(line)

    insert_index = len(preserved)

    new_block: list[str] = []
    for key in sorted(data.keys()):
        prop = f"{key}_ios"
        label = LABEL_MAP.get(key, f"{key.replace('_', ' ').title()} iOS")
        new_block.append(f"{prop}.type = bool")
        new_block.append(f"{prop}.label = {label}")
        new_block.append(f"{prop}.private = 1")
        new_block.append("")
    if new_block and new_block[-1] == "":
        new_block.pop()

    merged = preserved
    if merged and merged[-1] != "":
        merged.append("")
    merged.extend(new_block)
    merged = collapse_blank_lines(merged)
    EXT_PROPERTIES.write_text("\n".join(merged) + "\n")


def main() -> None:
    current_version = read_current_ios_version()
    main_html = fetch(MAIN_URL)
    frame_url = find_frame_url(main_html)
    frame_html = fetch(frame_url)
    data = parse_frame(frame_html)
    sdk_info = fetch_latest_ios_sdk(current_version)
    OUTPUT.write_text(render_podfile(data, sdk_info))
    rewrite_ext_properties(data)
    print(f"Wrote {OUTPUT} and updated ext.properties for {len(data)} adapters.")
    notes = sdk_info.get("notes")
    if notes:
        msg = f"iOS SDK version {sdk_info['version']} (release notes excerpt, see {RELNOTES_URL}):"
        print(msg)
        for entry in notes:
            print(f"- {entry['version']}:")
            for item in entry["items"]:
                print(f"  • {item}")
    else:
        if sdk_info.get("notes_unavailable", False):
            print(f"iOS SDK version {sdk_info['version']} (no newer release notes found on the page; see {RELNOTES_URL})")
        else:
            print(f"iOS SDK version {sdk_info['version']} (see {RELNOTES_URL})")


if __name__ == "__main__":
    main()
