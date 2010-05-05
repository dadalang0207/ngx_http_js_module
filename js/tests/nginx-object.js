;(function(){

NginxTests.nginxObject = function (r)
{
	r.sendHttpHeader('text/plain; charset=utf-8')
	
	var args = UrlEncode.parse(r.args)
	
	Tests.test('tests for Nginx object', function (t)
	{
		t.type(Nginx, 'object', 'Nginx object')
		t.type(Nginx.logError, 'function', 'Nginx.logError')
		t.eq(Nginx.prefix, args.prefix, 'compare Nginx.prefix and args.prefix')
		t.eq(Nginx.pid, +args.pid, 'Nginx.pid')
		
		t.test('Nginx.time', function (t)
		{
			var timerPrecision = 250
			
			var jmsec = +new Date(),
				nmsec = Nginx.time
			
			t.peq(jmsec, nmsec, timerPrecision, 'new Date() == Nginx.time')
			
			function later (e)
			{
				var ljmsec = +new Date(),
					lnmsec = Nginx.time
				
				t.peq(ljmsec, lnmsec, timerPrecision, 'new Date() == Nginx.time')
				t.peq(ljmsec - jmsec, timerPrecision * 4, timerPrecision, 'diff within one second')
			}
			t.async(later, timerPrecision * 4)
		})
	})
	Tests.oncomplete = function ()
	{
		r.sendSpecial(Nginx.HTTP_LAST)
	}
	Tests.run(r)
	
	return Nginx.OK
}

})();