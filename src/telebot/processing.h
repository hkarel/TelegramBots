#pragma once

#include "tele_data.h"

#include "shared/list.h"
#include "shared/defmac.h"
#include "shared/steady_timer.h"
#include "shared/qt/qthreadex.h"

#include <QtCore>
#include <QNetworkReply>
#include <atomic>

namespace tbot {

/**
  Параметры для функции sendTgCommand()
*/
struct TgCommandParams
{
    // Список параметров для Телеграм API функций
    QMap<QString, QVariant> api;

    // Задержка отправки команды, задается в миллисекундах
    int delay = {0};

    // Номер попытки выполняемого запроса
    int attempt = {1};

    // Определяет тайм-аут для удаления сервисных сообщений бота, задается
    // в секундах. Если значение тайм-аута равно 0 сообщение будет удалено
    // немедленно, таким образом оно останется только в истории сообщений.
    // При значении параметра меньше 0 сообщение не удаляется
    int messageDel = {0};
};

class Processing : public QThreadEx
{
public:
    typedef lst::List<Processing, lst::CompareItemDummy> List;

    Processing();

    bool init(qint64 botUserId, const QString& commandPrefix);
    static void addUpdate(const QByteArray&);

signals:
    void sendTgCommand(const QString& funcName, const tbot::TgCommandParams&);

    // Обработка штрафов за спам-сообщения
    void reportSpam(qint64 chatId, const tbot::User::Ptr&);

    // Аннулирование штрафов за спам-сообщения
    void resetSpam(qint64 chatId, qint64 userId);

    void updateBotCommands();
    void updateChatAdminInfo(qint64 chatId);

public slots:
    void reloadConfig();

private:
    Q_OBJECT
    DISABLE_DEFAULT_COPY(Processing)

    void run() override;

    // Обрабатывает команды для бота
    bool botCommand(const tbot::Update&);

private:
    static QMutex _threadLock;
    static QWaitCondition _threadCond;
    static QList<QByteArray> _updates;

    volatile bool _configChanged = {true};

    struct MediaGroup
    {
        // Одно из сообщений медиа-группы плохое
        bool isBad = {false};

        // Идентификатор чата для сообщений медиа-группы
        qint64 chatId = {0};
        QSet<qint64> messageIds;

        // Таймер для очистки _mediaGroups
        steady_timer timer;

    };
    static QMap<QString /*media_group_id*/, MediaGroup> _mediaGroups;

    bool _spamIsActive;
    QString _spamMessage;

    qint64 _botUserId = {0};
    QString _commandPrefix;
};

} // namespace tbot
