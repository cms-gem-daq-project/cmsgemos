#!/bin/sh -xe

# Thanks to:
# https://djw8605.github.io/2016/05/03/building-centos-packages-on-travisci/
# https://github.com/opensciencegrid/htcondor-ce/tree/master/tests

# Version of CentOS/RHEL
el_version=$1

# Run tests in Container
if [ "$el_version" = "6" ]
then
    echo "Running SLC6 custom docker image"
    docker_image=gitlab-registry.cern.ch/sturdy/gemdaq_ci_worker:slc6
    # docker_image=cern/slc6-base
    sudo docker run --entrypoint="" ${docker_image} /bin/bash -c "bash -xe ./.travis/docker.sh ${OS_VERSION}"
elif [ "$el_version" = "7" ]
then
    echo "Running CC7 custom docker image"
    docker_image=gitlab-registry.cern.ch/sturdy/gemdaq_ci_worker:cc7
    # docker_image=cern/cc7-base
    docker run --privileged -d -ti -e "container=docker"  -v /sys/fs/cgroup:/sys/fs/cgroup $docker_image /usr/sbin/init
    DOCKER_CONTAINER_ID=$(docker ps | grep centos | awk '{print $1}')
    docker logs $DOCKER_CONTAINER_ID
    docker exec -ti $DOCKER_CONTAINER_ID /bin/bash -xec "bash -xe ./.travis/docker.sh ${OS_VERSION};
  echo -ne \"------\nEND CMSGEMOS TESTS\n\";"
    docker ps -a
    docker stop $DOCKER_CONTAINER_ID
    docker rm -v $DOCKER_CONTAINER_ID
fi
