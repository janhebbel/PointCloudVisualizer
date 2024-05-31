# PointCloudVisualizer
A collection of programs with different approaches but the same function: collecting depth images, calculating their point cloud and rendering the result to the screen. For the sake of testing and comparing their performance against each other.

![Screenshot 2024-05-19 173755](https://github.com/jhebbel/PointCloudVisualizer/assets/75396907/b10fffad-d2f4-4445-bbfc-6b7d377412e2)
![Screenshot 2024-05-19 173802](https://github.com/jhebbel/PointCloudVisualizer/assets/75396907/663a93ee-d93b-4b9d-8acf-08af31874cfc)

## Dependencies
- Azure Kinect SDK
- GLFW
- OpenCL
- PCL

## Build Instructions
### Windows
For the Azure Kinect version:
- Download and install the [Azure Kinect SDK](https://learn.microsoft.com/en-us/azure/kinect-dk/sensor-sdk-download).
- Navigate to the installation path (for example: C:\Program Files\Azure Kinect SDK v1.4.1) and copy the 'k4a.lib'.
- Back in the directory of this repository navigate to 'AzureKinect' and then, for each version (CPU-based, OpenGL, ...), create a folder 'lib' and put the 'k4a.lib' inside this folder.

For every version (Azure Kinect & epc660) except the 'CPU-based' and 'PCL' ones:
- Download the [GLFW binaries](https://www.glfw.org/download/).
- Copy the 'glfw3_mt.lib' and put it into the same 'lib' folder we created before.

For both OpenCL versions (Azure Kinect & epc660):
- Download the 'OpenCL-SDK-vYYYY.MM.DD-Win-x64.zip' from https://github.com/KhronosGroup/OpenCL-SDK/releases
- Copy the 'OpenCL.lib' to the 'lib' folder.

For both PCL versions:
- Download the 'PCL-1.14.1-AllInOne-msvc2022-win64.exe' from https://github.com/PointCloudLibrary/pcl/releases
- Install the PCL using the downloaded installer.
- Then, for both PCL versions head to AzureKinect/PCL/ create the folder 'build' then open a command prompt cd into build and call 'cmake ..'

After having downloaded everything and putting everything in its proper place. Just call the build.bat for the version that you want to compile. If you want to compile multiple versions at once there are build_all.bat files in every parent directory.

### Linux
#### In general
Install the Azure Kinect SDK (only Ubuntu 18.04), download GLFW3, download OpenCL SDK and download the PCL.

#### For Debian-based distributions:

For the Azure Kinect version (only Ubuntu 18.04 supported):
- Follow the instructions from here: https://learn.microsoft.com/en-us/azure/kinect-dk/sensor-sdk-download#linux-installation-instructions and https://learn.microsoft.com/en-us/linux/packages#debian-based-linux-distributions. Summary below:
- curl -sSL -O https://packages.microsoft.com/config/ubuntu/18.04/packages-microsoft-prod.deb
- sudo dpkg -i packages-microsoft-prod.deb
- rm packages-microsoft-prod.deb
- sudo apt-get update
- sudo apt install k4a-tools
- sudo apt install libk4a<major>.<minor>-dev

For GLFW:
- Check if your Linux distributions package manager provides GLFW.
- For Debian-based distributions for example: sudo apt install libglfw3

For OpenCL:
- sudo apt install ocl-icd-opencl-dev

For PCL:
- sudo apt install libpcl-dev

After having downloaded everything and putting everything in its proper place. Just call the build.sh for the version that you want to compile. If you want to compile multiple versions at once there are build_all.sh files in every parent directory.

### Ethernet Settings for the epc660 Version
To be able to run any of the epc660 applications you will need make some changes to your ethernet settings:
- Using Windows navigate to your ethernet settings. Once there, edit your IP settings. At the top select Manual, turn IPv4 on. For the IP address enter: 192.168.10.1. For the Subnet prefix length enter 24. For the Gateway enter 192.168.10.0. And for the Preferred DNS enter 8.8.8.8. Press save.
