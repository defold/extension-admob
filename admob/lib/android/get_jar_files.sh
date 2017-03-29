
TMP=./tmp
mkdir $TMP

RES=../../res/android
mkdir -p $RES

function UnpackJar {
	local name=$1
	local base=$(basename $1 ".jar")
	base=$(basename $base ".aar")
	echo NAME $name - $base

	if [ -d $TMP/$base ];
	then
		echo $TMP/$base already exists
	else
		unzip -q $name -d $TMP/$base
	fi

	ls $TMP/$base/res

	if [ -e $TMP/$base/classes.jar ]
	then
		cp -v $TMP/$base/classes.jar ./$base.jar
	fi

	# copy resources if they exist
	if [ -d $TMP/$base/res ];
	then
		if [[ $base == *"play-services"* ]]; then
			cp -v -r $TMP/$base/res $RES
		fi
	fi
}

#################################

../gradlew fatJar

for f in build/one-jar-build/lib/*.aar build/one-jar-build/lib/*.jar
#for f in build/one-jar-build/lib/*.jar
do
	UnpackJar $f
	echo 
done


#UnpackJar build/one-jar-build/lib/firebase-ads-10.2.0.aar
#UnpackJar build/one-jar-build/lib/firebase-analytics-10.2.0.aar