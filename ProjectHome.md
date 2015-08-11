# Keymacs #
WindowsでEmacsライクキーバンドを実現するユーティリティ


---


## ■Keymacsとは？ ##

KeymacsはWindowsのキーアサインをemacs風にするためのユーティリティです。

つまり，Keymacsを使えば あのメモ帳がMuleみたいに使えるかもしれません...

アプリケーションごとに有効/無効を切り替えられます。

## ■スナップショット ##
![http://keymacs.googlecode.com/svn/wiki/images/screenshot.png](http://keymacs.googlecode.com/svn/wiki/images/screenshot.png)

## ■こんなあなたにKeymacsはぴったり!! ##

  * 普段はLinux使ってるけど，学校じゃ仕方なくWindowsを使ってる。
  * 上司がM$ Wordで書いた書類しか受け取ってくれない。
  * IEでCtrl+Hとか押すとヒストリが出て，死ねとか思う。
  * 当然Caps LockとCtrlキーの位置は入れ替えてある。
  * 左手の小指がよくつる。

## ■ダウンロード ##

  * バイナリ [[keymacs-0.211.zip(9KB)](https://keymacs.googlecode.com/files/keymacs-0.211.zip)]
  * ソース [[keymacs-src-0.211.zip(20KB)](https://keymacs.googlecode.com/files/keymacs-src-0.211.zip)]


## ■FAQ ##

  * Q. Ctrl+Spaceでマークセットを利用しているときに、挙動がおかしくなるんですが。
  * A. この現象は、KeymasがマークセットをShiftキー押しっぱなし で実現するという、 素敵に強引な仕様を持っているためです。 解決方法は、Shiftキーを一回押すか、Ctrl+Gで、Shift押しっぱなしが解除されるので直ると思います。

## ■Keymacs関連ページ ##

  * xkeymacs -- Yutaka OishiさんによるKeymacsクローン。普通の人はこっちを使いましょう(^^;

## ■使用上の注意 ##

KeymacsはGPLに基づいて配布されています。

## ■更新履歴 ##

| version 0.211 | 1999/12/21 | Ctrl+Sで検索を追加。そんだけ。 |
|:--------------|:-----------|:-------------------|
| version 0.21  | 1999/11/20 | 一回終了しないと設定が保存されないバグの修正。 [Ctrl+Iでタブ] の追加。 無視リストを行選択できるようにした。 なんとなく設定用プロパティーシートの配置を変えてみた。 |
| version 0.20  | 1999/11/15 | 無視リスト機能の追加。        |
| version 0.12  | 1999/11/05 | PowerPoint等での不具合修正。[適用]ボタンの追加。 |
| version 0.11  | 1999/11/04 | いくつかバグ修正。          |
| version 0.10  | 1999/11/03 | 大幅に作り直し。見た目は地味になる。 |
| version 0.01  | 1999/11/01 | 勢いで公開。             |