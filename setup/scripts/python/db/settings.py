import os
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "db.settings.local")
SECRET_KEY = 'ef7$_)r0$#(x)ovd^m_6_bevsonkaxalvrhds9@ilb_p0o&-xn'

INSTALLED_APPS = [
    'ldqm_db',
    ]
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.mysql',
        'NAME': 'ldqm_test_db',
        'PORT': 3306,
        'HOST': 'gem904daq01',
        'USER': 'gemdaq',
        'PASSWORD': 'gemdaq',
    }
}


