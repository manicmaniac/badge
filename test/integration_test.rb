require 'minitest/autorun'
require 'open3'

class IntegrationTest < Minitest::Test
  def test_that_running_with_version_option_shows_version
    stdout, stderr, status = Open3.capture3('badge', '--version')
    assert_includes(stdout, Badge::VERSION)
    assert_equal(stderr, '')
    assert_equal(status, 0)
  end

  def test_that_running_with_help_option_shows_help
    stdout, stderr, status = Open3.capture3('badge', '--help')
    assert_includes(stdout, 'badge')
    assert_equal(stderr, '')
    assert_equal(status, 0)
  end
end
