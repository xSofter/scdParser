# scdParser
ICD/CID xml file parse for IEC61850 protocol

# 修订说明
## 2020-11-23
1 增加对站控层全站CRC校验信息的处理
2 增加对Communication Private PortMap的记录
3 增加函数调用的DeBug打印
4 解决tag和value解析时,字符串残留的问题

## 2020-11-24
1 增加站控层GOOSE发送端口配置的解析
2 增加private节点的portmap解析
3 增加告警 日志级别处理
4 增加CONST 宏定义
5 增加AccessPoint层,将S1和G1的LN节点区分,将LD逻辑设备层放入该层中
   