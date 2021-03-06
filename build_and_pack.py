import os, sys
import glob
import re
import zipfile
import subprocess
import shutil

def fatal(error_msg):
    print(error_msg)
    sys.exit(-1)

def empty_folder_if_exist(folder_path):
    print("Unpack to " + folder_path)
    if os.path.exists(folder_path):
        print("Path exists already, emptying it")
        for the_file in os.listdir(folder_path):
            file_path = os.path.join(folder_path, the_file)
            try:
                if os.path.isfile(file_path):
                    os.unlink(file_path)
                elif os.path.isdir(file_path):
                    shutil.rmtree(file_path)
            except Exception as e:
                print(e)

def main(args):
    # Are we invoked by teamcity? Then 3 zip packages should be available
    # vali-<version>-<arch>.zip
    # vali-sdk-<version>-<arch>.zip
    # vali-ddk-<version>-<arch>.zip
    sdk_path = os.path.join(os.getcwd(), 'vali-sdk')
    ddk_path = os.path.join(os.getcwd(), 'vali-ddk')
    app_path = os.path.join(os.getcwd(), 'vali-apps')
    
    main_zip_regex = re.compile('vali-([0-9]+).([0-9]+).([0-9]+)-([0-9a-zA-Z]+).zip', re.IGNORECASE)
    sdk_zip_regex = re.compile('vali-sdk-([0-9]+).([0-9]+).([0-9]+)-([0-9a-zA-Z]+).zip', re.IGNORECASE)
    ddk_zip_regex = re.compile('vali-ddk-([0-9]+).([0-9]+).([0-9]+)-([0-9a-zA-Z]+).zip', re.IGNORECASE)
    version_parts = []
    arch = ''
  
    for file in glob.glob('*.zip'):
        print("Detected zip file: " + file)
        m = main_zip_regex.match(file)
        if m:
            print("Detected primary zip file")
            version_parts.append(m.group(1))
            version_parts.append(m.group(2))
            version_parts.append(m.group(3))
            arch = m.group(4)
            os.unlink(file)
        else:
            m = sdk_zip_regex.match(file)
            if m:
                print("Detected sdk zip file")
                empty_folder_if_exist(sdk_path)
                sdk_zip = zipfile.ZipFile(file, 'r')
                sdk_zip.extractall(sdk_path)
                sdk_zip.close()
                os.unlink(file)
            else:
                m = ddk_zip_regex.match(file)
                if m:
                    print("Detected ddk zip file")
                    empty_folder_if_exist(ddk_path)
                    ddk_zip = zipfile.ZipFile(file, 'r')
                    ddk_zip.extractall(ddk_path)
                    ddk_zip.close()
                    os.unlink(file)
                else:
                    print("Detected unusued zip file, removing")
                    os.unlink(file)
    
    # Spawn the makefile process
    version_arg = "VALI_VERSION=" + version_parts[0] + "." + version_parts[1] + "." + version_parts[2]
    sdk_arg = "VALI_SDK_PATH=" + sdk_path
    ddk_arg = "VALI_DDK_PATH=" + ddk_path
    app_arg = "VALI_APPLICATION_PATH=" + app_path
    p = subprocess.Popen(['make', 'package', version_arg, sdk_arg, ddk_arg, app_arg])
    p.wait();
  
if __name__== "__main__":
    main(sys.argv)