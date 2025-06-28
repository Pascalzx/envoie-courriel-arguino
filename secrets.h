// Configuration secrète - NE PAS COMMITER CE FICHIER
#ifndef SECRETS_H
#define SECRETS_H

// Configuration Wi-Fi
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Configuration SMTP Mailgun
#define SMTP_HOST "smtp.mailgun.org"
#define SMTP_PORT 587
#define SMTP_USER "postmaster@YOUR_DOMAIN.mailgun.org"
#define SMTP_PASS "YOUR_MAILGUN_PASSWORD"

// Configuration email
#define EMAIL_FROM "mailbox@YOUR_DOMAIN.mailgun.org"
#define EMAIL_TO "pascal@example.com"
#define MAILGUN_DOMAIN "YOUR_DOMAIN.mailgun.org"

// Configuration NTP
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC -18000  // UTC-5 (EST)
#define DAYLIGHT_OFFSET_SEC 3600  // +1h pour EDT

#endif