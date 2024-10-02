#ifndef _mount_HPP_INCLUDED_
#define _mount_HPP_INCLUDED_

#include <iostream>
#include <cstdio> // For popen function
#include <string>
#include <sstream>

bool isDeviceMounted(const std::string &device)
{
    std::string lsblkCommand = "lsblk -o NAME,MOUNTPOINT";

    // Open pipe to lsblk command
    FILE *lsblkPipe = popen(lsblkCommand.c_str(), "r");
    if (!lsblkPipe)
    {
        std::cerr << "Error executing lsblk command." << std::endl;
        return false;
    }

    // Read lsblk command output line by line
    char buffer[128];
    std::string line;
    bool deviceFound = false;
    while (fgets(buffer, sizeof(buffer), lsblkPipe) != nullptr)
    {
        line = buffer;

        // Check if the line contains the device name
        if (line.find(device) != std::string::npos)
        {
            deviceFound = true;

            // Check if the device is mounted
            if (line.find(" /") != std::string::npos)
            {
                // Device is mounted
                pclose(lsblkPipe);
                return true;
            }
            else
            {
                // Device is not mounted
                pclose(lsblkPipe);
                return false;
            }
        }
    }

    // Device not found
    pclose(lsblkPipe);
    return false;
}

bool mountAndCheck(const char *source, const char *target)
{
    if (isDeviceMounted(source))
    {
        std::cout << "Device " << source << " is already mounted." << std::endl;
        return true;
    }

    std::string mountCommand = "sudo mount ";
    mountCommand += source;
    mountCommand += " ";
    mountCommand += target;

    int status = system(mountCommand.c_str());
    if (status == 0)
    {
        std::cout << "Mount successful." << std::endl;
        return true; // Mount was successful
    }
    else
    {
        std::cout << "Mount failed." << std::endl;
        return false; // Mount failed
    }
}

#endif /*_mount_HPP_INCLUDED_*/