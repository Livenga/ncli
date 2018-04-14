# ncli

コマンド
```
./ncli --domain www.example.com --service https --path / -v
```

応答
```
* アクセス先: www.example.com//:https
[www.example.com]: 93.184.216.34(443)
* status:  200
* version: 1.1
** cache-control: max-age=604800
** content-type: text/html
** date: sat, 14 apr 2018 04:48:20 gmt
** etag: "1541025663+gzip+ident"
** expires: sat, 21 apr 2018 04:48:20 gmt
** last-modified: fri, 09 aug 2013 23:54:35 gmt
** server: ecs (sjc/4e39)
** vary: accept-encoding
** x-cache: hit
** content-length: 1270
* 読み込みモード: Content-Length
<!doctype html>
<html>
<head>
...
以下, レスポンス
```

#### TODO
- SSL クライアント証明書(指定なし) 場合によってはエラー
