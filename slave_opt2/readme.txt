#### 运行 

```bash

chmod u+x ./run.sh
./run.sh

```


#### 各文件内容描述
| 文件名                               | 描述                                     |
| :----------------------------------- | :--------------------------------------- |
| log                                  | 存放运行结果的目录                       |
| athread_7_y_ldm_prefetch_reg.c       | stensil7 salve 实现 计算内层             |
| athread_7_y_ldm_reg.c                | stensil 7 slave 实现 用于计算外层上下面  |
| athread_27_y_ldm_prefetch_reg_simd.c | stensil 27 slave 实现  计算内层          |
| athread_27_y_ldm_reg_simd.c          | stensil 27 slave 实现 用于计算外层上下面 |
| athread_halo_7_z_ldm_reg.c           | stensil 7 slave 实现 用于计算外层前后面  |
| athread_halo_27_z_ldm_reg_simd.c     | stensil 27 slave 实现 用于计算外层前后面 |
| benchmark.c                          | main文件 启动算法 计时 不可修改          |
| check.c                              | 检查算法正确性                           |
| common.h                             | 公共头文件                               |
| comp_utils.h                         | slave 计算工具函数声明                   |
| comp_utils.c                         | slave 计算工具函数实现                   |
| cpcrun                               | 打榜提交工具                             |
| host_common.c                        | host 工具函数实现                        |
| Makefile                             | Makefile                                 |
| mpi_comm_helper.c                    | mpi 通信工具函数实现                     |
| run.sh                               | 运行程序脚本                             |
| slave_comm.h                         | 从核通信工具函数声明                     |
| slave_comm.c                         | 从核通信工具函数实现                     |
| stencil-naive.c                      | stencil host 实现原始版本                |
| stencil-optimized.c                  | stencil host 实现优化版本                |



