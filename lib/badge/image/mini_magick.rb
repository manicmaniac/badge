require "mini_magick"
require "badge/image/error"

class Image
  def self.open(url)
    new(MiniMagick::Image.open(url))
  rescue MiniMagick::Invalid => e
    raise InvalidImage.new(e)
  end

  def self.load(path)
    new(MiniMagick::Image.new(path))
  rescue MiniMagick::Invalid => e
    raise InvalidImage.new(e)
  end

  def initialize(image)
    @image = image
  end

  def colorspace(colorspace)
    @image.colorspace(colorspace)
  end

  def format(format)
    @image.format(format)
  end

  def resize(width, height, strategy = nil)
    case strategy
    when :shrink
      Image::new(@image.resize("#{width}x#{height}>"))
    when :enlarge
      Image::new(@image.resize("#{width}x#{height}<"))
    else
      Image::new(@image.resize("#{width}x#{height}"))
    end
  end

  def write(full_path)
    @image.write(full_path)
  end

  def path
    @image.path
  end

  def width
    @image.width
  end

  def height
    @image.height
  end

  def composite(overlay, alpha_channel, gravity, geometry = nil)
    result = @image.composite(overlay, 'png') do |c|
      c.compose "Over"
      c.alpha 'On' unless !alpha_channel
      c.gravity gravity
      c.geometry geometry if geometry
    end
    Image::new(result)
  end
end
