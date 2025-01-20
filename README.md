# simple-termux-bash-scripts
A simple repo with curious Termux bash scripts

```sh
wget -qO- https://raw.githubusercontent.com/C1ENC14SOD4/simple-termux-bash-scripts/refs/heads/main/proot > ./proot
```

Give it execution permissions and setup at Termux start up

```bash
chmod +x ./proot
mv ./proot ~/.termux/start/proot
echo 'source "$HOME/.termux/start/proot" >> $HOME/.bashrc'
```
