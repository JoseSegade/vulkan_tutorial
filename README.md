# vulkan_tutorial
Vulkan tutorial based on GetIntoGameDev: 

https://www.youtube.com/@GetIntoGameDev

### Get Vulkan SDK - Add environment variables
To run vulkan it is necessary the SDK provided by the official site:
[Download the last version of the Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

Extract the tar file:
```shell
tar xf vulkansdk-linux-x86_64-1.x.yy.z.tar.xy
```

Move the directory created to a known directory. In my case it is inside `$HOME/vulkan/`

Inside the directory named after the version, that you just unpacked, give the `.sh` file permission to execute:
```shell
chmod +x ./setup-env.sh
```

Execute the `.sh` script to load the environment variables:
```shell
source ./setup-env.sh
```

You can check if the environment variable has been created with the following command:
```shell
printenv
```
