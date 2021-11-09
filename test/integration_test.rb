require 'minitest/autorun'
require 'fileutils'
require 'open3'
require 'shellwords'

class IntegrationTest < Minitest::Test
  EXECUTABLE_PATH = File.expand_path('../bin/badge', __dir__)
  ASSETS_DIR = File.expand_path('../assets', __dir__)
  ICON_FILENAME = 'icon175x175.png'
  IMAGE_DIFF_OUTPUT_DIR = File.expand_path('../tmp/image_diffs', __dir__)

  def initialize(name = nil)
    @name = name
    super(name) unless name.nil?
  end

  def setup
    @working_dir = Dir.mktmpdir
    FileUtils.cp(File.join(ASSETS_DIR, ICON_FILENAME), @working_dir)
    @previous_working_dir = Dir.pwd
    Dir.chdir(@working_dir)
  end

  def teardown
    Dir.chdir(@previous_working_dir)
    FileUtils.remove_entry(@working_dir)
  end

  # badge --version
  def test_that_running_with_version_option_shows_version
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--version')
    assert_includes(stdout, Badge::VERSION)
    assert_empty(stderr)
    assert_equal(0, status)
  end

  # badge --help
  def test_that_running_with_help_option_shows_help
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--help')
    assert_includes(stdout, 'badge')
    assert_empty(stderr)
    assert_equal(0, status)
  end

  # badge
  def test_that_running_with_no_option_adds_light_badge
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_light_badged.png'))
  end

  # badge --alpha
  def test_that_running_with_alpha_option_adds_alpha_light_badge
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--alpha', '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_alpha_light_badged.png'))
  end

  # badge --dark
  def test_that_running_with_dark_option_adds_dark_badge
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--dark', '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_dark_badged.png'))
  end

  # badge --shield "Version-0.0.3-blue" --dark
  def test_that_running_with_shield_dark_option_adds_blue_shield
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--shield', 'Version-0.0.3-blue', '--dark', '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_shield_Version-0.0.3-blue.png'))
  end

  # badge --shield "1.2-2031-orange" --no_badge
  def test_that_running_with_shield_no_badge_option_adds_shield_without_badge
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--shield', '1.2-2031-orange', '--no_badge', '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_shield_1.2-2031-orange.png'))
  end

  # badge --shield "1.2-2031-orange" --no_badge --shield_no_resize
  def test_that_running_with_shield_no_resize_option_adds_shield_without_resizing
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--shield', '1.2-2031-orange', '--no_badge', '--shield_no_resize', '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_shield_1.2-2031-orange-no-resize.png'))
  end

  # badge --shield "Version-0.0.3-blue" --dark --shield_geometry "+0+25%" --shield_scale 0.75
  def test_that_running_with_shield_geometry_scale_option_adds_shield_in_specified_position
    stdout, stderr, status = Open3.capture3(EXECUTABLE_PATH, '--shield', 'Version-0.0.3-blue', '--dark', '--shield_geometry', '+0+25%', '--shield_scale', '0.75', '--glob', "/#{ICON_FILENAME}")
    assert_includes(stdout, 'Start adding badges...')
    assert_empty(stderr)
    assert_equal(0, status)
    assert_images_equal(ICON_FILENAME, File.join(ASSETS_DIR, 'icon175x175_shield_Version-0.0.3-blue-geo-scale.png'))
  end

  private

  def assert_images_equal(expected, actual, threshold = 0.2, save_diff = true)
    if save_diff
      FileUtils.mkdir_p(IMAGE_DIFF_OUTPUT_DIR) if save_diff
      FileUtils.cp(expected, File.join(IMAGE_DIFF_OUTPUT_DIR, "#{@name}-expected.png"))
      output = File.join(IMAGE_DIFF_OUTPUT_DIR, "#{@name}-diff.png")
    else
      output = 'null:'
    end
    _, stderr, status = Open3.capture3('compare', '-metric', 'AE', '-verbose', '-dissimilarity-threshold', threshold.to_s, actual, expected, output)
    FileUtils.cp(actual, File.join(IMAGE_DIFF_OUTPUT_DIR, "#{@name}-actual.png")) if save_diff
    assert_equal(0, status.to_i, stderr)
  end
end
