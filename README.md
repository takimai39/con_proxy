con_proxy
====

簡単な、proxy プログラムです。
単純に指定したポートへ接続された通信内容を、リモートホストの指定ポートへ転送します。

## Compile 

$ cc con_proxy.c -o con_proxy -lpthread

## Usage

$ con_proxy
con_proxy: local-port remote-host remote-port 
$ 

$ con_proxy 10023 192.168.1.23 23 
Waiting for contact ...  
                    ^^^^^ Send ^^^^^ 
0000: ff fd 18 ff  fd 20 ff fd  23 ff fd 27               | ..... ..#..' 
                    VVVVV Receive VVVVV 
0000: ff fb 18 ff  fb 20 ff fb  23 ff fb 27               | ..... ..#..' 
                    ^^^^^ Send ^^^^^ 
0000: ff fa 20 01  ff f0 ff fa  23 01 ff f0  ff fa 27 01  | .. .....#.....'. 
0010: ff f0 ff fa  18 01 ff f0                            | ........ 
... 略 ... 

この例では、ローカルポート 10023 へ接続してきた通信を、192.168.1.23 の 23 番（telnetポート）に接続します。
telnet localhost 10023 と、自ホストの 10023 に接続すると、192.168.1.23 の telnet へ接続されます。

## 利用用途
例えば JDBC 接続間に、この proxy を経由させるなどを行うと、転送されているデータがリアルタイムで画面上に表示され確認する事ができます。
また、シンプルな内容ですので特定の条件を加えて、スリープで遅延させたり、強制的に通信を切断するなどのエラー確認のためにも利用できます。

何かのお役に立ちましたら幸いです。
