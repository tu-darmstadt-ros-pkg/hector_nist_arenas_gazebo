#include <QApplication>
#include <hector_nist_arena_designer/ui/editor.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Editor w;
    int success = 0;

    if (w.loadingSuccessful())
    {
        w.show();
        success = a.exec();
    }

    return success;
}
