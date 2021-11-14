#!/usr/bin/env bash

deploy_type=""
local_dir=""
APP_BASE_NAME=`basename "$0"`
MAVEN_LOCAL_DEPOLY_DIR="file:${HOME}/.m2/repository"

set -ex

if [ -n "$1" ]; then
    deploy_type=$1
fi

getRelative() {
    input=$*
    array=(${input//:/ })

    size=${#array[@]}
    if [ ${size} -eq 1 ]; then
        local_dir=${array[0]}
    else
        local_dir=${array[1]}
    fi
}

deploy2local() {
    DIR=$*

    getRelative ${DIR}
    if [ ! -d ${local_dir} ]; then
      mkdir -p ${local_dir}
    fi

    ./gradlew clean uploadArchives \
    -PSNAPSHOT_REPOSITORY_URL=${DIR} -PRELEASE_REPOSITORY_URL=${DIR}
}

deploy2sonatype() {
    ./gradlew clean uploadArchives --refresh-dependencies
}

case "$deploy_type" in
  sonatype* )
    deploy2sonatype
    ;;
  mavenLocal* )
    deploy2local $MAVEN_LOCAL_DEPOLY_DIR
  ;;
  * )
    echo "Usage: $APP_BASE_NAME [sonatype|mavenLocal]"
esac
