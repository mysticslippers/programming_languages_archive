import unittest
import subprocess


def launch(word):
    process = subprocess.Popen(
        ["./lab"],
        text=True,
        shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    stdout, stderr = process.communicate(input=word)
    return process.returncode, stdout.strip(), stderr.strip()


class Tester(unittest.TestCase):

    def test_input_available(self):
        tests = [["third", 0, "world!", ""], ["second", 0, "another", ""], ["first", 0, "Hello", ""],
                 ["asdahskdjkashasdkad", 1, "", "Not found!"], ["a" * 256, 1, "", "Word is too long for buffer!"]]

        for test in tests:
            returnCode, returnOut, returnErr = launch(test[0])
            self.assertEqual(returnCode, test[1])
            self.assertEqual(returnOut, test[2])
            self.assertEqual(returnErr, test[3])


if __name__ == "__main__":
    unittest.main()
