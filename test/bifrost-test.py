import sys
import os
import subprocess
import json

type = sys.argv[1]
bin_path = sys.argv[2]

def print_test(str):
  print(f"[TEST]: {str}")

if type == "test_and_load":
  print_test("Running test_and_load")
  injector_exe_path = os.path.join(bin_path, "injector.exe")
  
  # Load
  cmd = f"{injector_exe_path} --json load --exe=01-hello-world.exe --exe-arg=60000 --plugin=01-hello-world-plugin.dll:name --no-wait"
  print_test(f"Launching {cmd}")
  proc = subprocess.Popen(cmd.split(' '), cwd=bin_path, stdout=subprocess.PIPE, shell=True)
  o,e = proc.communicate()
  output = json.loads(o)
  
  # Unload
  remote_pid = output["Output"]["RemoteProcessPid"]
  smem_name = output["Output"]["SharedMemoryName"]
  smem_size = output["Output"]["SharedMemorySize"]
  cmd = f"{injector_exe_path} --json load --pid={remote_pid} --plugin=name --shared-memory-name={smem_name} --shared-memory-size={smem_size} --no-wait"
  print_test(f"Launching {cmd}")
  proc = subprocess.Popen(cmd.split(' '), cwd=bin_path, stdout=subprocess.PIPE, shell=True)
  proc.wait()
  
  print_test("Done test_and_load")
  