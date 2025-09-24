# level0->level10
## level0
测试ssh连接：ssh -p 2220 bandit0@bandit.labs.overthewire.org
## level0->level1
![alt text](47f24bae6a1a01ea4d6617b56e6110ae.png)
## level1->level2
![alt text](b37f6f5160143eb2138119f56dafdd46.png)
使用./保护文件名（cd - 表示上次cd）
## level2->level3
![alt text](9cff7133f9a707c7c01e3adf17a6c0de.png)
--表示后面没有参数，""用来保护文件完整（''也可以）
## level3->level4
![alt text](46584ccf00d21aeb3acb480787b25f3a.png)
ll寻找隐藏的文件
## level4->level5
![alt text](34b34492a7de6ac502403b3d715b651c.png)
枚举寻找可读文件
## level5->level6
![alt text](0c7ac0739576939fed375d1c4c194987.png)
使用find函数，maxdepth表示搜索深度
## level6->level7
![alt text](e8389026008abf6f5bfbd55cc23c362c.png)
2>dev/null是将错误重定向
## level7->level8
![alt text](efba47a27e43f137c0fba7feee58c590.png)
在less窗口输入/再输入字符串和回车可以定位，用n和N向下或向上搜索
## level8->level9
![alt text](98955c18ddb30853bb69a21094f5e6e6.png)
sort先排序uniq再筛选，-u参数表示只出现过一次
## level9->level10
![alt text](a20912566d8db435ebddb5ada4ea2ddf.png)
strings命令用于从二进制文件中提取可打印的字符串，-a表示整个文件，grep正则进行匹配
## level10->level11
![alt text](04f51ebc86f41635ec1060e928d77c41.png)
使用base64（密文位数是4的倍数，有=）解码，-d,解码数据,-i,在解码时，忽略非字母表字符（即丢弃非法 Base64 字符）