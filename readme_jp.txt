【MQBoneManagerについて】
Metasequoia 4.5.6上でボーンにアクセスするための暫定ライブラリです。
現在MQBoneManagerはβ版として提供されます。ライブラリのAPIおよび作成したプ
ラグインについては今後のバージョンでの互換性は保証されません。
そのため、作成したプラグインについては社内など特定の相手のみへの配布を行う
ことはできますが、Web上など不特定の相手を対象とした配布はできません。
一般向け配布については今後MQBoneManagerがプラグインSDKに正式に取り込まれる
までお待ちください。


【サンプルプロジェクトのビルドについて】
ExportPMDのビルドには外部ライブラリtinyxml2が必要です。
tinyxml2を
https://github.com/leethomason/tinyxml2
からダウンロードし、ExportPMD\tinyxml2フォルダに
  tinyxml2.cpp
  tinyxml2.h
を配置してください。

