##############################################
#!/bin/bash
##############################################

mkdir -p /etc/vespa/config
mkdir -p /etc/vespa/cert/server
mkdir -p /etc/vespa/cert/client
mkdir -p /etc/vespa/cert/ext-client

#if running stop VESPa agent service

if pgrep -x "VESPa" > /dev/null
then
	echo "Found a running instance of VESPa, Stoping now .. "
	killall VESPa
	echo "Installing VESPa and configurations .."
else
	echo "Installing VESPa and configurations .."
fi

cp app/bin/* /bin/
#Copy any config to /etc dir
cp eula.txt /etc/vespa/
cp version /etc/vespa/
cp vespa.local /etc/vespa/config/
	
echo "Installation complete"

