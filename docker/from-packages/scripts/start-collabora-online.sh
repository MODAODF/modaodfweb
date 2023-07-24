#!/bin/sh
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if test "${DONT_GEN_SSL_CERT-set}" = set; then
# Generate new SSL certificate instead of using the default
mkdir -p /tmp/ssl/
cd /tmp/ssl/
mkdir -p certs/ca
openssl rand -writerand /opt/modaodfweb/.rnd
openssl genrsa -out certs/ca/root.key.pem 2048
openssl req -x509 -new -nodes -key certs/ca/root.key.pem -days 9131 -out certs/ca/root.crt.pem -subj "/C=DE/ST=BW/L=Stuttgart/O=Dummy Authority/CN=Dummy Authority"
mkdir -p certs/servers
mkdir -p certs/tmp
mkdir -p certs/servers/localhost
openssl genrsa -out certs/servers/localhost/privkey.pem 2048
if test "${cert_domain-set}" = set; then
openssl req -key certs/servers/localhost/privkey.pem -new -sha256 -out certs/tmp/localhost.csr.pem -subj "/C=DE/ST=BW/L=Stuttgart/O=Dummy Authority/CN=localhost"
else
openssl req -key certs/servers/localhost/privkey.pem -new -sha256 -out certs/tmp/localhost.csr.pem -subj "/C=DE/ST=BW/L=Stuttgart/O=Dummy Authority/CN=${cert_domain}"
fi
openssl x509 -req -in certs/tmp/localhost.csr.pem -CA certs/ca/root.crt.pem -CAkey certs/ca/root.key.pem -CAcreateserial -out certs/servers/localhost/cert.pem -days 9131
mv -f certs/servers/localhost/privkey.pem /etc/modaodfweb/key.pem
mv -f certs/servers/localhost/cert.pem /etc/modaodfweb/cert.pem
mv -f certs/ca/root.crt.pem /etc/modaodfweb/ca-chain.cert.pem
fi

# Disable warning/info messages of LOKit by default
if test "${SAL_LOG-set}" = set; then
SAL_LOG="-INFO-WARN"
fi

# Replace trusted host and set admin username and password - only if they are set
if test -n "${aliasgroup1}" -o -n "${domain}" -o -n "${remoteconfigurl}"; then
    perl -w /start-collabora-online.pl || { exit 1; }
fi
if test -n "${username}"; then
    perl -pi -e "s/<username (.*)>.*<\/username>/<username \1>${username}<\/username>/" /etc/modaodfweb/modaodfweb.xml
fi
if test -n "${password}"; then
    perl -pi -e "s/<password (.*)>.*<\/password>/<password \1>${password}<\/password>/" /etc/modaodfweb/modaodfweb.xml
fi
if test -n "${server_name}"; then
    perl -pi -e "s/<server_name (.*)>.*<\/server_name>/<server_name \1>${server_name}<\/server_name>/" /etc/modaodfweb/modaodfweb.xml
fi
if test -n "${dictionaries}"; then
    perl -pi -e "s/<allowed_languages (.*)>.*<\/allowed_languages>/<allowed_languages \1>${dictionaries:-de_DE en_GB en_US es_ES fr_FR it nl pt_BR pt_PT ru}<\/allowed_languages>/" /etc/modaodfweb/modaodfweb.xml
fi

# Restart when /etc/modaodfweb/modaodfweb.xml changes
[ -x /usr/bin/inotifywait -a -x /usr/bin/killall ] && (
  /usr/bin/inotifywait -e modify /etc/modaodfweb/modaodfweb.xml
  echo "$(ls -l /etc/modaodfweb/modaodfweb.xml) modified --> restarting"
  /usr/bin/killall -1 modaodfweb
) &

# Generate WOPI proof key
modaodfweblconfig generate-proof-key

# Start modaodfweb
exec /usr/bin/modaodfweb --version --o:sys_template_path=/opt/modaodfweb/systemplate --o:child_root_path=/opt/modaodfweb/child-roots --o:file_server_root_path=/usr/share/modaodfweb --o:logging.color=false ${extra_params}
