<h1 align="center" style="font-weight: 900; font-size: 30px;" >About</h1>
Telegram bot for tracking new posts by tags for rule34 gelbooru, kemono, pixiv, with the ability to filter

<h1 align="center" style="font-weight: 900; font-size: 30px;" >Settings</h1>

In the environment variable specify the token of the Telegram bot in TOKEN

example:
```
TOKEN=0000000:fdgdfgdfghdjhjkrhtjeo
```
<h1 align="center" style="font-weight: 900; font-size: 30px;" >Guide</h1>

After launching the bot you need to configure it, first you need to add yourself or someone else as a user to use the bot

Admin Command:

```
/adduser <id>
/deluser <id>
```

After adding a user, he/she has access to commands

User Command:

```
/help
```

Add/remove tag to search for new posts
```
/addtag <service> <tag>
/deltag <service> <tag>
```
Add/Remove tag with which posts will not be sent to you
```
/addantitag <service> <tag>
/delantitag <service> <tag>
```
list of tags you added
```
/taglist
```

**Please note the way to add tags for kemono is different from others**
```
/addtag kemono <service>/<id>
/deltag kemono <service>/<id>
```
example:
```
https://kemono.su/fanbox/user/000000
fanbox - service
000000 - id

/addtag kemono fanbox/000000
```
