import json

def application(environ, start_response):
	status = '200 OK'
	output = environ
	
	response_headers = [('Content-type', 'application/json'),
						('Content-Length', str(len(output)))]
	start_response(status, response_headers)
	
	return [output]
