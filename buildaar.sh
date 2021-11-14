#!/usr/bin/env bash

echo "BuildAAR:init"

if [ $# == 1 ] ; then
echo "参数正确"
else
echo '参数错误'
echo '必选参数:'
echo '   1 string 类型 build type'
echo './buildaar.sh mavenLocal'
exit 1;
fi

echo "BuildAAR::start build type  $1"


# '备份 setting.gradle'
cp settings.gradle settings.gradle.backup
echo "" > settings.gradle
IFS=',' read -r -a moduleArr <<< "xxlog"

for module in "${moduleArr[@]}"
do
echo "$module";
echo "include ':$module'" >> settings.gradle
done

if [ "$(uname)" == "Darwin" ]
then
    sed -i '' "s/^devXXLog=.*/devXXLog=0/g" gradle.properties
else
    sed -i "s/^devXXLog=.*/devXXLog=0/g" gradle.properties
fi

./deploy.sh $1

pwd
echo 'BuildAAR::Success!'

#恢复setting.gradle文件
mv settings.gradle.backup settings.gradle
