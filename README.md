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
   
## 2020-11-27

1 对底层链表遍历的优化处理,访问每个LN节点详细内容时,采用本级LN作为遍历的起点,实现灵活遍历的结果  
类似  
``` xml
<LDevice inst="PROT" desc=" ">
    <LN0 desc=" " lnType=" " lnClass="LLN0" inst="">
        <DataSet name="dsTripParaInfo" desc="故障量数据集">
            <FCDA ldInst="PROT" lnClass="RFLO" lnInst="1" doName="FltVPhA" fc="MX"/>
            <FCDA ldInst="PROT" lnClass="RFLO" lnInst="1" doName="FltVPhB" fc="MX"/>
            <FCDA ldInst="PROT" lnClass="RFLO" lnInst="1" doName="FltVPhC" fc="MX"/>
        </DataSet>
```
2 增加对LN节点下,屏蔽处理不应该存在的Elements  
3 对DOI节点下的DAI,以及SDI节点下的DAI做拆分处理,减少解析短地址的工作量  

## 2020-12-03 
1 支持DOI节点下,递归解析SDI和DAI节点  
2 在解析完每个LDevice后,找到DataSet的每个FCDA地址,并将addr和desc和type记录下来  
3 支持记录每个DAI的name和描述  
4 解决日志记录级别不正确的问题,无法记录LOG_Error的问题  

## 2020-12-09 
1 增加SclPub 文件,提供用户获取接口,封装加载和销毁内存接口  
2 提供解析fcda短地址功能的接口,增加提供解析通信配置的接口  
3 解决部分字符串处理接口bug  
4 删除部分过程调试打印  

## 2020-12-16 
1 解决nd_chk_strdup 申请内存时,未判空导致赋值字符串出现段错误  
2 结构化存储解析后的LN节点信息,将多层链表展开为2维表格进行储存  
3 增加PUB文件的API接口,提供更多解析功能  
4 增加ustorage源文件,将解析支持结果存储在sqlit3和mySql数据库中  
5 解决解析scd文件中出现可选字段不存在时,解析停止的问题  

## 2020-12-22  
1 支持数据库存储通信、buffer和非buffer报告、支持存储log信息  
2 优化用户信息展示对内存的占用  
3 通信信息表支持GSE SMV的APPID VLAN的查看  
4 支持查看scl文件的IED信息,crc和全站CRC信息  
5 API的异常流程增加日志和指针保护处理  
6 支持解析多个IED的功能  

## 2020-12-25 
1 支持模型文件解析结果保存一份在用户日志中  
2 删除解析ix的内容,目前不再需要  
3 解决解析dastaSet过程中,对某些SDI类型的节点,无法解析短地址的问题,修改方法改为根据LN类型,判断doiname和fc类型相匹配的DAI  
4 解决不需要的调试打印信息,不关注的信息使用INFO级别日志输出  
5 解决strncpy_safe接口存在空指针拷贝的隐患  
6 解决解析大型SCD文件过程中,出现部分属性字段不存在时,解析停止的问题