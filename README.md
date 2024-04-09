# RTED home task

## Task N1
In order to run the script please open terminal and enter the following code
``` sh
/workspace/run_scripts/build_projects.sh
/workspace/task_n1/run_task_n1.sh
```

## Task N2
In order to run the script please open 3 terminals and enter the following code

### terminal N.1
``` sh
/workspace/run_scripts/build_projects.sh
/workspace/task_n2/run_server_proc.sh
```

### terminal N.2
``` sh
/workspace/task_n2/run_bbg_proc.sh
```

### terminal N.3
``` sh
/workspace/task_n2/run_stm32_proc.sh
```

## Notes
- there is a known bug regarding the logging og the total cost of parking in the pango task. had now time to fix it
- there is a known bug regarding the clearing of shared memory in the bbg process. it works well on my computer but does not work in the devcontainer environment