#!/bin/sh

function ShowHelp()
{
    echo "Useage : project_creator.sh project_name [path]"
}

if [ $# -lt 1 ] ; then
    ShowHelp
    exit
fi
    
project_name=$1
project_path=.

if [ $# -ge 2 ] ; then
    project_path=$2
fi

echo "========================================================================"
echo "Creating Project [${project_name}] On Directory [${project_path}]"
echo "========================================================================"

if [ ! -d ${project_path} ] ; then
    mkdir ${project_path} -p
fi

if [ -d ${project_path}/${project_name} ] ; then
    echo "${project_path}/${project_name} Aleady exist, Please choose other name"
    rm -rf ${project_path}/${project_name}
    #exit
fi


tools_path=`pwd -P`

cd ${project_path}
mkdir ${project_name}
cd ${project_name}

project_full_path=`pwd -P`

#echo "project_full_path = ${project_full_path}"
#echo "tools_path = ${tools_path}"

# copy header source file to include
mkdir include
cp ${tools_path}/../elegance include/ -R
for cpp_file in `find include | grep "cpp$"`; do
    rm $cpp_file
done

# copy library
cp ${tools_path}/../lib . -R


# copy src and vs project files
cp ${tools_path}/project_creator/src . -R
cp ${tools_path}/project_creator/VS2015_project . -R
cp ${tools_path}/project_creator/CMakeLists.txt .


sed -e "s/EleganceHelloworld/${project_name}/g" -i VS2015_project/EleganceHelloworld*
sed -e "s/EleganceHelloworld/${project_name}/g" -i CMakeLists.txt

cd VS2015_project/
mv EleganceHelloworld.sln ${project_name}.sln
mv EleganceHelloworld.vcxproj ${project_name}.vcxproj
mv EleganceHelloworld.vcxproj.filters ${project_name}.vcxproj.filters


echo
echo "project has succesfull created on :"
echo "${project_full_path}"
echo "========================================================================"
