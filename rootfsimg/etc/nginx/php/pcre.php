<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Regex Matcher</title>
</head>
<body>
    <h1>Regex Matcher</h1>
    <form method="POST">
        <label for="regex">Enter Regular Expression:</label><br>
        <input type="text" id="regex" name="regex" required style="width: 100%;" placeholder="e.g., /[aeiou]/"><br><br>

        <label for="content">Enter Content:</label><br>
        <textarea id="content" name="content" required style="width: 100%; height: 200px;" placeholder="e.g. The cat and the hat."></textarea><br><br>

        <button type="submit">Match</button>
    </form>

    <hr>

    <?php
        $regex = ngx_post_args()['regex'];
        $content = ngx_post_args()['content'];

        if (isset($regex) && isset($content)) {
            echo "<h2>Results</h2>";

            $regex = urldecode($regex);
            $content = urldecode($content);

            var_dump($regex);
            var_dump($content);

            // Validate the regex format
            //@preg_match($regex, "") === 
            if (@preg_match($regex, "") === false) {
                echo "<p style='color: red;'>Invalid regular expression.</p>";
            } else {
                if (preg_match_all($regex, $content, $matches)) {
                    echo "<p style='color: green;'>Matches found:</p>";
                    echo "<ul>";
                    foreach ($matches[0] as $match) {
                        echo "<li>" . htmlspecialchars($match) . "</li>";
                    }
                    echo "</ul>";
                } else {
                    echo "<p style='color: red;'>No matches found.</p>";
                }
            }
        }


    ?>
</body>
</html>