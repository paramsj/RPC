<img width="1402" height="316" alt="image" src="https://github.com/user-attachments/assets/5830c241-f291-4ece-9059-5cc853447364" />


how to run?
clear the build dir first as that might cause some issues

1) cmake .. -DCMAKE_PREFIX_PATH=/usr/local 
MAKE SURE THAT PROTOC AND GRPC ARE LOCATED HERE

2) make

3) as usual start client and server in their respective termninals

i cannot get this to work in post man however
