open Odiff.ImageIO;
open Odiff.Diff;

let getIOModule = filename =>
  Filename.extension(filename)
  |> (
    fun
    | ".png" => ((module ODiffIO.Png.IO): (module ImageIO))
    | ".jpg"
    | ".jpeg" => ((module ODiffIO.Jpg.IO): (module ImageIO))
    | ".bmp" => ((module ODiffIO.Bmp.IO): (module ImageIO))
    | ".tiff" => ((module ODiffIO.Tiff.IO): (module ImageIO))
    | f => failwith("This format is not supported: " ++ f)
  );

type diffResult('output) = {
  exitCode: int,
  diff: option('output),
};

let main =
    (
      img1Path,
      img2Path,
      diffPath,
      threshold,
      outputDiffMask,
      failOnLayoutChange,
      diffColorHex,
      stdoutParsableString,
      antialiasing,
      ignoreRegions,
    ) => {
  module IO1 = (val getIOModule(img1Path));
  module IO2 = (val getIOModule(img2Path));

  module Diff = MakeDiff(IO1, IO2);

  let img1 = IO1.loadImage(img1Path);
  let img2 = IO2.loadImage(img2Path);

  let {diff, exitCode} =
    Diff.diff(
      img1,
      img2,
      ~outputDiffMask,
      ~threshold,
      ~failOnLayoutChange,
      ~antialiasing,
      ~ignoreRegions,
      ~diffPixel=
        Color.ofHexString(diffColorHex)
        |> (
          fun
          | Some(col) => col
          | None => (255, 0, 0) // red
        ),
      (),
    )
    |> Print.printDiffResult(stdoutParsableString)
    |> (
      fun
      | Layout => {diff: None, exitCode: 21}
      | Pixel((diffOutput, diffCount, stdoutParsableString))
          when diffCount == 0 => {
          exitCode: 0,
          diff: Some(diffOutput),
        }
      | Pixel((diffOutput, diffCount, diffPercentage)) => {
          // IO1.saveImage(diffOutput, diffPath);
          {exitCode: 22, diff: Some(diffOutput)};
        }
    );

  IO1.freeImage(img1);
  IO2.freeImage(img2);

  switch (diff) {
  | Some(output) when outputDiffMask => IO1.freeImage(output)
  | _ => ()
  };

  exit(exitCode);
};
