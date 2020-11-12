<?php
header( 'Content-Type: application/json' );
// Read the static file or just generate a dynamic content.
@readfile( __DIR__ . '/schedule.json' );
exit;