#include <QApplication>
#include <QMainWindow>
#include <QFile>
#include <QScreen>
#include <QLabel>
#include <QPixmap>
#include <QSlider>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QPointer>

#define API_HOST "iot-light-bulb.rf.gd"

QPointer<QLabel> brightness_feedback_label;
QPointer<QLabel> light_bulb_label;
QPointer<QNetworkAccessManager> network_manager;

QString get_app_stylesheet()
{
    QFile qss_file(":/resources/application.qss");
    qss_file.open( QFile::ReadOnly );

    if(!qss_file.isOpen())
    {
        qCritical() << "Failed to load the css template";
        return QString();
    }

    return qss_file.readAll();
}

static inline int find_x_axis_center(const QWidget& parent, const QSize& child_size)
{
    return (parent.width()/2) - (child_size.width()/2);
}


void display_light_bulb_image(const int percent, QPointer<QLabel>& target_label)
{
    QPixmap light_bulb_image;

    if(percent < 20) {
        light_bulb_image.load(":/resources/light_bulb_low.png");
    }
    else if(percent < 50) {
        light_bulb_image.load(":/resources/light_bulb_medium.png");
    }
    else if(percent < 80) {
        light_bulb_image.load(":/resources/light_bulb_high.png");
    }
    else {
        light_bulb_image.load(":/resources/light_bulb_extreme.png");
    }

    target_label->setPixmap(light_bulb_image.scaled(target_label->width(),target_label->height(),Qt::KeepAspectRatio));
}

void display_brightness_percent(const int percent, QPointer<QLabel>& brightness_feedback_label, const QMainWindow *w)
{
    QString display_message = QString::number(percent);
    display_message.append(" %");
    brightness_feedback_label->setText(display_message);
    const QSize estimated_size = brightness_feedback_label->sizeHint();
    brightness_feedback_label->setGeometry(find_x_axis_center(*w,estimated_size),brightness_feedback_label->y(),estimated_size.width(),estimated_size.height());
}

void brightness_adjustment_bar_handler(int percent)
{
    display_brightness_percent(percent * 1.4, brightness_feedback_label, dynamic_cast<QMainWindow*>(brightness_feedback_label->parent()));
    display_light_bulb_image(percent, light_bulb_label);

    const int pwm_val = (percent/100.0) * 1023;

    QString url_path = "http://";
    url_path.append(API_HOST);
    url_path.append("/iot/light_bulb.php");

    QUrlQuery request_params;
    request_params.addQueryItem("update_pwm", QString::number(pwm_val));

    QNetworkRequest http_req((QUrl(url_path)));
    http_req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    //For some free web servers
    http_req.setHeader(QNetworkRequest::UserAgentHeader, "Googlebot/2.1 (+http://www.google.com/bot.html)");

    //Free memory took up by the requests
    network_manager->clearConnectionCache();

    network_manager->post(http_req,request_params.toString().toUtf8());
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QSize screen_size = app.primaryScreen()->availableSize();
    QMainWindow w;
    w.setGeometry(w.x(),w.y(),screen_size.width(),screen_size.height());

    app.setAttribute(Qt::AA_UseStyleSheetPropagationInWidgetStyles, true);
    app.setStyleSheet(get_app_stylesheet());

    QLabel app_title_label("Light bulb intensity controller",&w);
    app_title_label.setObjectName("app_title");
    const QSize pref_app_title_geometry = app_title_label.sizeHint();
    app_title_label.setGeometry(find_x_axis_center(w,pref_app_title_geometry), (w.height()/100.0)*20, pref_app_title_geometry.width(), pref_app_title_geometry.height());
    app_title_label.show();

    light_bulb_label = new QLabel(&w);
    const int light_bulb_img_width = (w.width()/100.0)*40;
    const QSize light_bulb_image_size(light_bulb_img_width, light_bulb_img_width / 1.17);
    light_bulb_label->setGeometry(find_x_axis_center(w,light_bulb_image_size), (w.height()/100.0)*35, light_bulb_image_size.width(), light_bulb_image_size.height());
    display_light_bulb_image(0,light_bulb_label);
    light_bulb_label->show();

    QSlider brightness_adjustment_bar(Qt::Horizontal, &w);
    QObject::connect(&brightness_adjustment_bar,&QSlider::valueChanged,brightness_adjustment_bar_handler);
    const int brightness_adjustment_bar_width = (w.width()/100.0)*75;
    const int brightness_adjustment_bar_y = light_bulb_label->y() + light_bulb_label->height() + (w.height()/100.0)*10;
    const QSize brightness_adjustment_bar_size(brightness_adjustment_bar_width, (w.height()/100.0)*2.5);
    brightness_adjustment_bar.setGeometry(find_x_axis_center(w,brightness_adjustment_bar_size), brightness_adjustment_bar_y , brightness_adjustment_bar_size.width(), brightness_adjustment_bar_size.height());
    brightness_adjustment_bar.show();

    brightness_feedback_label = new QLabel(&w);
    brightness_feedback_label->setObjectName("brightness_percentage");
    const int brightness_feedback_label_y = brightness_adjustment_bar.y() + brightness_adjustment_bar.height() + (w.height()/100.0)*2;
    brightness_feedback_label->setGeometry(0,brightness_feedback_label_y,0,0);
    display_brightness_percent(0, brightness_feedback_label, &w);

    network_manager = new QNetworkAccessManager(&w);
    QObject::connect(network_manager, &QNetworkAccessManager::finished, [](QNetworkReply *reply){
        reply->deleteLater();
    });

    w.show();
    return app.exec();
}


