import unittest
import subprocess


def launch(word: str):
    process = subprocess.Popen(
        "./lab",
        text=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = process.communicate(input=word)
    return process.returncode, stdout.strip(), stderr.strip()


class Tester(unittest.TestCase):

    def test_input_available(self):
        tests = [
            ("third", 0, "world!", ""),
            ("second", 0, "another", ""),
            ("first", 0, "Hello", ""),
            ("asdahskdjkashasdkad", 1, "", "Not found!"),
            ("a" * 256, 1, "", "Word is too long for buffer!"),
        ]

        for word, expected_code, expected_out, expected_err in tests:
            with self.subTest(word=word):
                return_code, return_out, return_err = launch(word)
                self.assertEqual(return_code, expected_code)
                self.assertEqual(return_out, expected_out)
                self.assertEqual(return_err, expected_err)


if __name__ == "__main__":
    unittest.main()