con_proxy
====

簡単な proxy プログラムです。
指定したローカルポートへの通信内容を、リモートホストの指定ポートへ転送します。
スレッド化していますので複数の通信も転送を行います。  
※ HP-UX, Linux , MacOS で動作確認済み。

## Compile 

$ cc con_proxy.c -o con_proxy -lpthread

## Usage

```
$ con_proxy
con_proxy: local-port remote-host remote-port  
$  
　
$ con_proxy 10023 192.168.1.23 23  
Waiting for contact ...   

この待ち状態で、別のターミナルから telnet localhost 10023 などで接続してみると通信は転送され同時に転送されているデータが hex dump されます。

                    ^^^^^ Send ^^^^^  
0000: ff fd 18 ff  fd 20 ff fd  23 ff fd 27               | ..... ..#..'  
                    VVVVV Receive VVVVV  
0000: ff fb 18 ff  fb 20 ff fb  23 ff fb 27               | ..... ..#..'  
                    ^^^^^ Send ^^^^^  
0000: ff fa 20 01  ff f0 ff fa  23 01 ff f0  ff fa 27 01  | .. .....#.....'.  
0010: ff f0 ff fa  18 01 ff f0                            | ........  
... 略 ...  
```
　

## 利用用途
例えば JDBC 接続間に、この proxy を経由させるなどすれば、転送されているデータがリアルタイムで画面上に表示され確認する事ができます。
また、シンプルな内容ですので特定の条件を加えて、スリープで遅延させたり、強制的に通信を切断するなどのエラー確認のためにも利用できます。

何かのお役に立ちましたら幸いです。
