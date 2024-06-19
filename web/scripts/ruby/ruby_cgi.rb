#!/usr/bin/ruby

require 'cgi'

cgi = CGI.new

# Recover the parameters
input = cgi['input_number'].to_i

puts cgi.header

# Make an svg image of a ruby inside a div element
def ruby_svg(i)
	if i.even?
		svg = '<svg xmlns="http://www.w3.org/2000/svg" aria-label="Ruby" role="img" viewBox="0 0 512 512"
		onclick="handleClick(' + i.to_s + ')" style="filter: hue-rotate(' + (i * 50).to_s + 'deg);">
			<rect width="512" height="512" rx="15%" fill="#ffffff"/>
			<path d="M407.7 397.3l20.86-257.7L348.02 297l-196 118c80.86-5.675 171.2-11.87 255.7-17.66z" fill="#a00403"/>
			<path d="M423.39 178.977L320 98l-27 93c109.797 9.317 91.763-9.646 130.39-12.023zM192 282l136 43-35-134zm-89 19c44 139 55 169 89-19l-90 21zM276 68l97 1-53 29c-11-7-36-24-44-30z M85 246l-4 99 23-43z M298 85c26 26-1 89-53 140s-118 83-144 57c-25-25.89.7-90 52.85-141s119.2-82 144.2-56z" fill="#b11205"/>
			<path d="M192 282l133 43c-48 45-137.5 86.5-173 90z" fill="#9f0d02"/>
			<path d="M293 191l33 133c40-42 76-88 94-144zm114.7 206.3L387.02 245 326 324z" fill="#891102"/>
			<path d="M421 181c13-41 16-101-48-112l-53 29z" fill="#aa1401"/>
			<path d="M81 345c2 68 50 69 71 70l-49-113z" fill="#9e1209"/>
			<path d="M192 282l54 103c32-17 57-38 79-61z" fill="#900e04"/>
			<path d="M103 302l-8 91c14 20 34 21 55 20-15-37-45-112-47-111z" fill="#8b1104"/>
		</svg>'
	else
		svg = '<svg id="ruby_svg_' + i.to_s + '" xmlns="http://www.w3.org/2000/svg" aria-label="Ruby" role="img" viewBox="0 0 512 512"
		onclick="rotateSvg(' + i.to_s + ')" style="filter: hue-rotate(' + (i * 50).to_s + 'deg);">
			<rect width="512" height="512" rx="15%" fill="#ffffff"/>
			<path d="M407.7 397.3l20.86-257.7L348.02 297l-196 118c80.86-5.675 171.2-11.87 255.7-17.66z" fill="#a00403"/>
			<path d="M423.39 178.977L320 98l-27 93c109.797 9.317 91.763-9.646 130.39-12.023zM192 282l136 43-35-134zm-89 19c44 139 55 169 89-19l-90 21zM276 68l97 1-53 29c-11-7-36-24-44-30z M85 246l-4 99 23-43z M298 85c26 26-1 89-53 140s-118 83-144 57c-25-25.89.7-90 52.85-141s119.2-82 144.2-56z" fill="#b11205"/>
			<path d="M192 282l133 43c-48 45-137.5 86.5-173 90z" fill="#9f0d02"/>
			<path d="M293 191l33 133c40-42 76-88 94-144zm114.7 206.3L387.02 245 326 324z" fill="#891102"/>
			<path d="M421 181c13-41 16-101-48-112l-53 29z" fill="#aa1401"/>
			<path d="M81 345c2 68 50 69 71 70l-49-113z" fill="#9e1209"/>
			<path d="M192 282l54 103c32-17 57-38 79-61z" fill="#900e04"/>
			<path d="M103 302l-8 91c14 20 34 21 55 20-15-37-45-112-47-111z" fill="#8b1104"/>
		</svg>'
	end
end

# Generate children elements of the div element inside a flex container
def div_children(input)
    children = '<div class="container">'
    input.times do |i|
        children += '<div style="display: flex; justify-content: center; align-items: center; width: 200px; height: 200px; border: 1px solid black; border-radius: 25px;">'
        children += ruby_svg(i)
        children += '</div>'
    end
    children += '</div>'
    children
end

# Output the HTML response
puts "<html>"
puts "
<head>
    <title>My First Ruby CGI Script</title>
    <style>
    * {
        font-family: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
    }
    body {
        background-color: rgb(140,115,124);
        display: flex;
        justify-content: center;
        align-items: center;
        height: 100vh;
        margin: 0;
        flex-direction: column;
    }
    h1 {
        font-size: 3em;
        color: white;
    }
    .container {
        display: flex;
        justify-content: center;
        flex-wrap: wrap;
        width: 100%;
    }
	.rotate {
        animation: rotation 2s infinite linear;
    }
    @keyframes rotation {
        from {
            transform: rotate(0deg);
        }
        to {
            transform: rotate(359deg);
        }
    }
    </style>
	<script>
    function handleClick(i) {
        alert('You clicked on ruby number ' + (i + 1));
    }
	function rotateSvg(i) {
        var svg = document.getElementById('ruby_svg_' + i);
        svg.classList.toggle('rotate');
    }
    </script>
</head>"
puts "<body>"
puts "<h1>Your Rubies!</h1>"
puts "<p>Amount: "
puts '<strong style="color: white">' + "#{input}</strong>"
puts "</p>"

puts div_children(input)

puts "</body>"
puts "</html>"