# mysqldump2spanner

MySQL の mysqldump コマンドで出力した内容を Google Spanner の形式に変換するツール  

## usage
```
[user@host]$ ./mysqldump2spanner.exe -h
Usage: ./mysqldump2spanner.exe [OPTIONS] < [FILE]

        -h      この使い方を表示して終了する
        -v      バージョン情報を表示して終了する
        -i num  INSERT 時の VALUE の数を制限 (default 1000, max 10000)
        -z      TIMESTAMP 列の値に付与するタイムゾーンを指定する (ex. Asia/Tokyo)
        -D      'DROP TABLE' を生成しません
        -F      'INTERLEAVE IN PARENT' を生成しません
        -I      'CREATE INDEX' を生成しません
```

## 開発環境

```
[user@host]$ uname -a
Linux Host1 3.10.0-957.10.1.el7.x86_64 #1 SMP Mon Mar 18 15:06:45 UTC 2019 x86_64 x86_64 x86_64 GNU/Linux
[user@host]$ cat /etc/redhat-release
CentOS Linux release 7.6.1810 (Core)
[user@host]$ rpm -q bison
bison-3.0.4-2.el7.x86_64
[user@host]$ rpm -q flex
flex-2.5.37-6.el7.x86_64
[user@host]$ rpm -q php
php-7.2.17-1.el7.remi.x86_64
[user@host] ]$ g++ --version
g++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-36)
Copyright (C) 2015 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

```

## コンパイル方法

```
[user@host]$ make
bison --report=all --report-file=report.out --output=parser.cc parser.yy
g++ -std=c++11 -Wall -Wextra -g -O0 -pipe -include std.hpp   -c -o parser.o parser.cc
flex --outfile=scanner.cc scanner.ll
g++ -std=c++11 -Wall -Wextra -g -O0 -pipe -include std.hpp   -c -o scanner.o scanner.cc
g++ -std=c++11 -Wall -Wextra -g -O0 -pipe -include std.hpp -c -o ddl.o ddl.cpp
g++ -std=c++11 -Wall -Wextra -g -O0 -pipe -include std.hpp -c -o main.o main.cpp

*** link ***
g++ -std=c++11 -Wall -Wextra -g -O0 -pipe -include std.hpp -o mysqldump2spanner.exe parser.o scanner.o ddl.o main.o
```

## 変換できるか確認

```
[user@host]$ MYSQL_PWD=migtest_pass mysql -umigtest_user -Dmigtest -e "show create table tab1\G"
*************************** 1. row ***************************
       Table: tab1
Create Table: CREATE TABLE `tab1` (
  `skey` varchar(10) NOT NULL,
  `data` varchar(100) NOT NULL,
  `num` int(11) DEFAULT NULL,
  `rate` double DEFAULT NULL,
  `flag` tinyint(1) DEFAULT NULL,
  `dtime` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`skey`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8

[user@host]$ MYSQL_PWD=migtest_pass mysqldump -umigtest_user migtest | ./mysqldump2spanner.exe > /dev/null
ddl-type=[drop] table=[tab1]

        * CONVERT MESSAGE
                W) IF EXISTS は無視されます

ddl-type=[create] table=[tab1]

        * COLUMN
                name=[skey] type=[VARCHAR(10)] options=[/NOT NULL/]
                name=[data] type=[VARCHAR(100)] options=[/NOT NULL/]
                name=[num] type=[INT(11)] options=[/DEFAULT(NULL)/]
                name=[rate] type=[DOUBLE] options=[/DEFAULT(NULL)/]
                name=[flag] type=[TINYINT(1)] options=[/DEFAULT(NULL)/]
                name=[dtime] type=[DATETIME] options=[/NOT NULL/DEFAULT(CURRENT_TIMESTAMP)/]

        * CONDITION
                type=[primary key] columns=[skey]

        * CONVERT MESSAGE
                I) column=[flag]: BOOL に変換されます
                W) column=[dtime]: DEFAULT(CURRENT_TIMESTAMP) は無視されます

ddl-type=[ignore] text=[LOCK TABLE tab1]

dml-type=[insert] table=[tab1]

        * COLUMN
                names=[skey,data,num,rate,flag,dtime]

        ! 3 record writed.

ddl-type=[ignore] text=[UNLOCK TABLES]

- Parse success.
```

## 変換実行

```
[user@host]$ MYSQL_PWD=migtest_pass mysqldump -umigtest_user migtest | ./mysqldump2spanner.exe 2> /dev/null
DROP TABLE tab1
;
CREATE TABLE tab1 (
  skey STRING(10) NOT NULL,
  data STRING(100) NOT NULL,
  num INT64,
  rate FLOAT64,
  flag BOOL,
  dtime TIMESTAMP NOT NULL,
) PRIMARY KEY(skey)
;
INSERT INTO (skey,data,num,rate,flag,dtime) VALUES ('A001','abc \' def \t hij \n klm',NULL,12097.664000,FALSE,'2019-04-17 23:06:13'),('A002','ABC\"',1,12097.664000,FALSE,'2019-04-17 23:06:13'),('A003','a\"',55,6.600000,FALSE,'2019-04-18 04:16:18')
;
```

## Spanner へのインポート (サンプル)

```
[user@host]$ MYSQL_PWD=migtest_pass mysqldump -umigtest_user migtest | ./mysqldump2spanner.exe -D > output.sql.txt 2> /dev/null
[user@host]$ php rebuild-spanner-db.php -i your-spanner-instance -d your-spanner-database -f output.sql.txt
Do you really want to drop the 'your-spanner-database' database [y/N] y
Database has been deleted.
Waiting for the database to be created... done.

        # DROP TABLE tab1

Waiting for DDL to complete... done.

        # CREATE TABLE tab1 (   skey STRING(10) NO...

Waiting for DDL to complete... done.

        # INSERT INTO tab1 (skey,data,num,rate,fla...

Execute DML... 3 row

all done.
```

