#ifndef DLG_KEY_H
#define DLG_KEY_H

#include <QDialog>
#include <QDebug>
#include <QKeyEvent>

#include "../util/shared.h"
#include "../core/translation_manager.h"

namespace Ui
{
	class dlg_key;
}

class dlg_key : public QDialog
{
Q_OBJECT

public:
	explicit dlg_key( unsigned int key, QWidget* parent = nullptr );
	~dlg_key( );

	unsigned int get_new_value( );

private slots:
	void on_cmdSave_clicked( );
	void on_cmdCancel_clicked( );
	void on_cmdPress_clicked( );

private:
	void update_text( );

protected:
	bool eventFilter( QObject* obj, QEvent* event );

private:
	Ui::dlg_key* ui;
	unsigned int m_curent_key;
	unsigned int m_new_key;
};

#endif // DLG_KEY_H
