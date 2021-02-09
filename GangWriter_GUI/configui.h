#ifndef CONFIGUI_H
#define CONFIGUI_H

#include <QWidget>
#include <QFileDialog>

namespace Ui {
class ConfigUi;
}

class ConfigUi : public QWidget
{
	Q_OBJECT

public:
	explicit ConfigUi(QWidget *parent = nullptr);
	~ConfigUi();

private slots:
	void on_btnExit_released();
	void on_btnOpenImage_released();
	void on_btnOpenOutDir_released();
	void on_btnBackUpDir_released();
	void on_btnSave_released();

private:
	Ui::ConfigUi *ui;

	void UpdateData();
};

#endif // CONFIGUI_H
