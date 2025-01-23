# simple-termux-bash-scripts
A simple repo with curious Termux bash scripts

Get the script (just copy and paste on terminal)

```sh
wget -qO- https://raw.githubusercontent.com/C1ENC14SOD4/simple-termux-bash-scripts/refs/heads/main/proot > ./proot
```

Give it execution permissions and setup at Termux start up

```bash
chmod +x ./proot
mkdir -p $HOME/.termux/start/
mv ./proot $HOME/.termux/start/proot
echo 'source "$HOME/.termux/start/proot" >> $HOME/.bashrc'
```
