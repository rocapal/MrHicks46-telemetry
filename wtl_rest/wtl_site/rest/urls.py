from django.conf.urls import patterns, include, url


urlpatterns = patterns('',
   
	url(r'^version/$', 'wtl_site.rest.views.version'),
	url(r'^dates/$', 'wtl_site.rest.views.dates'),

	url(r'^traces/(?P<date>(\d{4}-\d{2}-\d{2}))/$', 'wtl_site.rest.views.traces'),

	url(r'^traces/georss/(?P<date>(\d{4}-\d{2}-\d{2}))/$', 'wtl_site.rest.views.traces_georss'),
)
