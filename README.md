# microdatabaseの説明
#### 今回のプロジェクトは以下の3つのCファイルから構成されている

- file.c  
- datadef.c
- datamanip.c
- main.c

1. file.cの説明  
file.cはファイル操作を行うモジュールである。ファイルの新規作成や削除を行う。  

2. datadef.cの説明  
detadef.cはデータの定義を行うモジュールである。フィールドの情報を定義する。  

3. detamanip.cの説明  
detamanip.cはデータの操作を行うモジュールである。データの挿入や削除を行う。

4. main.cの説明  
main.cはメイン関数をのせたモジュールで、残り3つのモジュールの中の関数を呼び出して
実行する。簡単な字句解析、構文解析を行っている。