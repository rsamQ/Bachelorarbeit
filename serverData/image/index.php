<?php
header( 'Content-Type: application/bmp' );
// Read the static file or just generate a dynamic content.
@readfile( __DIR__ . '/image.bmp' );
exit;