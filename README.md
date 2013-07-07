# xpce を Flash に移植してみた

xpce 0.11a を Flash に移植してみました。  
http://www.geocities.co.jp/SiliconValley/7052/

動作確認用  
http://hkpr.info/flash/xpce/


概要
-----

* Flash Player 11 以降でないと動かないと思います。
* Flash Player が動作する環境であれば、 Windows 以外でも動くと思います。
* FlasCC 1.0.1 を使って、配布元のコードの Windows 依存部分以外を、ほぼそのままビルドしています。
* サウンドも再生されます。 Flash の再生環境によってはノイズが乗る可能性もありますので、ヘッドフォン等で再生される場合は気を付けてください。
* Core i5 4 Core の環境で、 CPU 使用率 10 % から 15 % で動作します。


今のところ対応していない機能
-----------------------------

* セーブできません。後々修正したいと思います。
* 各種設定画面がありません。
* 圧縮イメージは読み込めません。


FlasCC + FlashDevelop 環境でのビルド方法
-----------------------------------------

* FlasCC_1.0.1/run.bat から cygwin を起動する
* export LANG=C  
を入力する
* ソースコードの入っているディレクトリ (xpce011as) に cd で移動
* make FLASCC="/path/to/FlasCC_1.0.1/sdk" FLEX="/path/to/flex_sdk_4.6/"  
で make が通ると思います
* make 後、 build ディレクトリに SWC ファイルが作成されます
* FlashDevelop で flash ディレクトリを開き、出力された SWC ファイルへのパスを通してビルドしてください。


動作確認環境
-------------

* FlashDevelop 4.3.0
* Flex SDK 4.6
* Flash Player 11
* FlasCC 1.0.1


ライセンス
-----------

本家のコードには特に明記されていませんでしたが、 xpce, fpce のライセンスをよく読んでからご使用ください。


