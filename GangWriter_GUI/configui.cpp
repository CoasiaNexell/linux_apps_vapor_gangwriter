#include "configui.h"
#include "ui_configui.h"
#include "configfile.h"
#include "NetworkClient.h"

#define LOG_TAG	"[CONFIG_UI]"
#include "NxDbgMsg.h"

ConfigUi::ConfigUi(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ConfigUi)
{
	ui->setupUi(this);

	UpdateData();
}

ConfigUi::~ConfigUi()
{
	delete ui;
}

void ConfigUi::on_btnExit_released()
{
	close();
}

void ConfigUi::on_btnOpenImage_released()
{
	QString file = QFileDialog::getOpenFileName(this, "Select File", QDir::currentPath());
	ui->edtImageFile->setText(file);
}

void ConfigUi::on_btnOpenOutDir_released()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::currentPath(), QFileDialog::ShowDirsOnly);
	ui->edtOutDir->setText(dir);
}

void ConfigUi::on_btnBackUpDir_released()
{
	QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", QDir::currentPath(), QFileDialog::ShowDirsOnly);
	ui->edtBackupDir->setText(dir);
}

void ConfigUi::on_btnSave_released()
{
	ConfigFile cfgFile;
	cfgFile.SetModules        (ui->edtModules  ->text());
	cfgFile.SetConfigFileName (ui->edtCfgFile  ->text());
	cfgFile.SetImageFilename  (ui->edtImageFile->text());
	cfgFile.SetOutDirectory   (ui->edtOutDir   ->text());
	cfgFile.SetBackupDirecotry(ui->edtBackupDir->text());

	cfgFile.SetIPAddress(ui->edtIpAddress->text());
	cfgFile.SetNetMask  (ui->edtNetmask  ->text());
	cfgFile.SetGateway  (ui->edtGateway  ->text());
	cfgFile.SetDNS      (ui->edtDns      ->text());

	cfgFile.SetServerIpAddr   (ui->edtSvrIpAddress->text());
	cfgFile.SetTargetDirectory(ui->edtSvrTargetDir->text());
	cfgFile.SetMountDirectory (ui->edtMountDir    ->text());

	cfgFile.SaveConfig();
}

void ConfigUi::UpdateData()
{
	ConfigFile cfgFile;
	ui->edtCfgFile->setText(cfgFile.GetConfigFileName());
	ui->edtModules->setText(cfgFile.GetModules());
	ui->edtImageFile->setText(cfgFile.GetImageFilename());
	ui->edtOutDir->setText(cfgFile.GetOutDirectory());
	ui->edtBackupDir->setText(cfgFile.GetBackupDirecotry());

	ui->edtIpAddress->setText(cfgFile.GetIPAddress());
	ui->edtNetmask->setText(cfgFile.GetNetMask());
	ui->edtGateway->setText(cfgFile.GetGateway());
	ui->edtDns->setText(cfgFile.GetDNS());

	ui->edtSvrIpAddress->setText(cfgFile.GetServerIpAddr());
	ui->edtSvrTargetDir->setText(cfgFile.GetTargetDirectory());
	ui->edtMountDir->setText(cfgFile.GetMountDirectory());
}
