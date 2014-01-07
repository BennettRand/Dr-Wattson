var the_body = document.getElementsByTagName("body")[0];

// console.log(the_body.innerHTML);

function appendDevice(name)
{
	the_body.innerHTML += "<div class=\"device\"><h3>" + name + "</h3></div>";
}

appendDevice("Lorem");
appendDevice("Ipsum");
appendDevice("Dolor");
appendDevice("Sit");
appendDevice("Amet");

for (var i=0;i<100;i++)
{
	appendDevice(i.toString());
}