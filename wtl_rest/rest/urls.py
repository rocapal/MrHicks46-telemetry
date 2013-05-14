from django.conf.urls.defaults import *

urlpatterns = patterns('',

        (r'^version/$', 'wtl_rest.rest.views.version'),
        (r'^dates/$', 'wtl_rest.rest.views.dates'),

        (r'^traces/(?P<date>(\d{4}-\d{2}-\d{2}))/$', 'wtl_rest.rest.views.traces'),

        (r'^traces/georss/(?P<date>(\d{4}-\d{2}-\d{2}))/$', 'wtl_rest.rest.views.traces_georss'),
)

