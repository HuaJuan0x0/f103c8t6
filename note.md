**PC_MSG**
nFlag = 0
| nCh   | other | 0xFA  | 0x4D  | Ox43  | Ox55 |
| :---- | :---: | :---: | :---: | :---: | ---: |
| nFlag |   0   |   1   |   0   |   0   |    0 |
| nFlag |   0   |   1   |   2   |   0   |    0 |
| nFlag |   0   |   1   |   0   |   3   |    0 |
| nFlag |   0   |   1   |   0   |   0   |    4 |
nFlag = 4 后开始接收控制命令
pMem[0] = nlen
pMem[1:nLen]

**Parse_Date**
pMem[1]<<8 + pMem[2] = nId
| nId    |                        control |
| :----- | -----------------------------: |
| 0x3001 |                 开始测量，对时 |
| 0x4001 |                       停止指令 |
| 0x5001 | 相机模式：1自动模式，2手动模式 |
| 0x5002 |          相机拍照指令 手动触发 |
| 0x6001 |                   返回模式指令 |
| 0x8001 |                     变间隔模式 |
| 0x5003 |              触发间隔 最小0.1s |
