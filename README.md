#
# LeTS-Drive with SUMMIT simulator integration
## Setup
### Pre-requisites
1. Install [CUDA 10.0](https://developer.nvidia.com/cuda-10.0-download-archive) (Note: you need to follow the [official guide](https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html) for a successful installation.)
2. Install [CUDNN 7](https://docs.nvidia.com/deeplearning/sdk/cudnn-install/index.html)
### Python Environment Setup
Set up a python virtualenv for an isolated setup of python packages which do not interfere with global python packages.
```
cd
virtualenv --system-site-packes lets_drive 
source ~/lets_drive/bin/activate
```
Optionally, append the source ~/lets_drive... line to ~/.bashrc. Also, to deactivate the current virtual environment, just type 
```
deactivate
```
### Setup Dependencies
Download all bash scripts in the setup folder, then run
```
bash setup.sh
```
This setup script will:
* install ros-melodic
* build and install the lastest libtorch (the CPP frontend of Pytorch)
* build and install OpenCV 4.1.0
* install dependent python packages
The script will prompt for sudo privilege.

### Setup the SUMMIT simulator
Download the [SUMMIT simultator](https://www.dropbox.com/s/3cnjktij8vtfn56/summit.zip?dl=0), and unzip it to, for example ~/summit

## Run the System
### Launch the Simulator
```
export SDL_VIDEODRIVER=offscreen
LinuxNoEditor/CarlaUE4.sh -carla-rpc-port=2000 -carla-streaming-port=2000
```
### Launch the Planner
```
cd ~/catkin_ws/src/scripts
./experiment_summmit.sh [gpu_id] [start_round] [end_round(inclusive)] [carla_portal, e.g. 2000]
```
experiment_summmit.sh does not record bags by default. To enable rosbag recording, change the following variable to 1 in the script:
```
record_bags=0
```
## Data Processing
Convert bags into h5 files using multiple threads:
```
python Data_processing/parallel_parse.py --bagspath [rosbag/path/] --peds_goal_path Maps/
```
or using a single thread:
```
python Data_processing/bag_to_hdf5.py --bagspath [rosbag/path/] --peds_goal_path Maps/
```
combine bag_h5 files into training, validation, and test sets:
```
python Data_processing/combine.py --bagspath [rosbag/path/]
```
## IL Training
Start training and open tensorboard port
```
python train.py --batch_size 512 --lr 0.01 --train train.h5 --val val.h5
tensorboard --logdir runs --port=6001
```
To check the learning curve, run on your Local machine:
```
ssh -4 -N -f -L localhost:6001:localhost:6001 panpan@unicorn4.d2.comp.nus.edu.sg
```
On your browser, visit http://localhost:6010/

**Main Arguments**:
- `train`: The path to the train dataset
- `test`: The path to the test dataset
- `bagspath`: Path to bag files
- `peds_goal_path`: Path to pedestrian goals
- `lr`: Learning rate 
- `epochs`: Number of epochs to train. Default: 100
- `k`: Number of Value Iterations. Recommended: [10 for 8x8, 20 for 16x16, 36 for 28x28]
- `l_i`: Number of channels in input layer. Default: 2, i.e. obstacles image and goal image.
- `l_h`: Number of channels in first convolutional layer. Default: 150, described in paper.
- `l_q`: Number of channels in q layer (~actions) in VI-module. Default: 10, described in paper.
- `batch_size`: Batch size. 



