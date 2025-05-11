#include "USARTAss.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);
	USARTAss w;
	w.show();
	return a.exec();
}