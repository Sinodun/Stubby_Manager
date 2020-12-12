#include "serverdatadialog.h"
#include "ui_serverdatadialog.h"

#include <QHostAddress>


ServerDataDialog::ServerDataDialog(Config::Server& server, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serverdatadialog),
    m_server(server)
{
    ui->setupUi(this);
    ui->serverNameData->setText(server.name.c_str());
    ui->serverWebsiteData->setText(server.link.c_str());
    ui->serverAddressData->setText(server.addresses[0].c_str());
    //ui->serverAddressData->setInputMask("000.000.000.000;");
    ui->serverAuthName->setText(server.tlsAuthName.c_str());
    ui->serverPinData->setText(server.pubKeyDigestValue.c_str());
    ui->v6AddressLabel->setVisible(false);
    ui->v6AddressEdit->setVisible(false);
    // remove question mark from the title bar
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->serverNameData->setToolTip("(Required) Set a user friendly name for this server");
    ui->serverWebsiteData->setToolTip("Set a link to a website with information about this server/service");
    ui->serverAddressData->setToolTip("(Required) The IPv4 Address for the server (IPv6 support coming soon)");
    ui->serverAuthName->setToolTip("Specify the TLS authentication name to use when authenticating this server");
    ui->serverPinData->setToolTip("Specify the pin (SPKI Digest Value to use when authenticating this server");
}

ServerDataDialog::~ServerDataDialog()
{
    delete ui;
}

void ServerDataDialog::on_applyButton_clicked() {
    qInfo("Validating server");
    bool result = true;
    QString message;

    // server MUST have a name
    if (ui->serverNameData->text().isEmpty()) {
        message.append("ERRROR: Server must have a name\n");
        result = false;
    }

    // server MUST have valid IP v4 address for now
    int pos;
    QString addr = ui->serverAddressData->text();
    QRegExp rx("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    validator = new QRegExpValidator(rx, this);
    if (validator->validate(addr, pos) != QValidator::Acceptable) {
        message.append("ERRROR: The Main IPv4 address is not a valid format");
        result = false;
    }
    //TODO: Add warning if no auth info

    if (result) {
        m_server.name              = ui->serverNameData->text().toUtf8().constData();
        m_server.link              = ui->serverWebsiteData->text().toUtf8().constData();;
        m_server.addresses[0]      = ui->serverAddressData->text().toUtf8().constData();;
        m_server.tlsAuthName       = ui->serverAuthName->text().toUtf8().constData();;
        m_server.pubKeyDigestValue = ui->serverPinData->text().toUtf8().constData();;
        accept();
    }
    else {
        ui->validationMessage->setText(message);
        return;
    }
}

void ServerDataDialog::on_discardButton_clicked() {
    reject();
}
