0 0 */1 * * root find /var/cache/ndcodfweb -type f -a -atime +10 -exec rm {} \;
