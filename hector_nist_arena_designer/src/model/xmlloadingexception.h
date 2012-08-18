#ifndef XMLLOADINGEXCEPTION_H
#define XMLLOADINGEXCEPTION_H

#include <exception>
#include <QString>
#include <QDomElement>

class XmlLoadingException : public std::exception
{
public:
    XmlLoadingException(const QString& msg, const QDomElement& node) throw()
        : m_msg(msg)
        , m_line(node.lineNumber())
        , m_col(node.columnNumber())
    {}
    ~XmlLoadingException() throw() {}

    QString msg() const { return m_msg; }
    int lineNumber() const { return m_line; }
    int columnNumber() const { return m_col; }

private:
    QString m_msg;
    int m_line;
    int m_col;
};

#endif // XMLLOADINGEXCEPTION_H
