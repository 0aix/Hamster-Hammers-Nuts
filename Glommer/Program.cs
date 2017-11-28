using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace Glommer
{
	class Program
	{
		static List<string> meshTOC = new List<string>();
		static List<string> sknTOC = new List<string>();
		static List<string> animTOC = new List<string>();
        static List<string> pngTOC = new List<string>();
        static int total_vertex_count = 0;
		static int total_bone_count = 0;
		static int total_xf_count = 0;
        static int total_png_size = 0;

		static void Main(string[] args)
		{
			string root = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location).TrimEnd('\\');

			Console.WriteLine("Writing hamster.glom");

			// hamster.glom
			FileStream hamster = File.Create(root + "\\hamster.glom");
			byte[] version = new byte[4];
			new Random().NextBytes(version);
			hamster.Write(version, 0, 4);

			MemoryStream meshbuffer = new MemoryStream();
			MemoryStream meshes = new MemoryStream();

			// MESH_BUFFER
			foreach (string s in Directory.EnumerateFiles(root + "\\meshes"))
			{
				FileInfo fi = new FileInfo(s);
				if (fi.Extension.Equals(".mesh"))
				{
					Console.WriteLine("Writing " + fi.Name);

					byte[] buffer = new byte[fi.Length];
					using (FileStream ifs = fi.OpenRead())
						ifs.Read(buffer, 0, buffer.Length);
					int vertex_count = BitConverter.ToInt32(buffer, 0);

					if (vertex_count * (3 * 4 + 3 * 4 + 3 * 4 + 2 * 4 + 4 * 4 + 4 * 4) + 4 != buffer.Length)
						throw new Exception(fi.Name + " file size incorrect");
					if (vertex_count % 3 != 0)
						throw new Exception(fi.Name + " vertex count not a multiple of 3");

					meshbuffer.Write(buffer, 4, buffer.Length - 4);
					meshbuffer.Flush();
					meshes.Write(BitConverter.GetBytes(total_vertex_count), 0, 4);
					meshes.Write(BitConverter.GetBytes(vertex_count), 0, 4);
					meshes.Flush();
					total_vertex_count += vertex_count;
					meshTOC.Add(fi.Name.Replace('.', '_').ToUpperInvariant());
				}
			}
			hamster.Write(BitConverter.GetBytes(total_vertex_count), 0, 4);
			hamster.Write(meshbuffer.ToArray(), 0, (int)meshbuffer.Length);
			hamster.Flush();
			meshbuffer.Close();

			// MESH
			hamster.Write(BitConverter.GetBytes(meshTOC.Count), 0, 4);
			hamster.Write(meshes.ToArray(), 0, (int)meshes.Length);
			hamster.Flush();
			meshes.Close();

			MemoryStream sknbuffer = new MemoryStream();
			MemoryStream skns = new MemoryStream();

			// SKN_BUFFER
			foreach (string s in Directory.EnumerateFiles(root + "\\skeletons"))
			{
				FileInfo fi = new FileInfo(s);
				if (fi.Extension.Equals(".skn"))
				{
					Console.WriteLine("Writing " + fi.Name);

					byte[] buffer = new byte[fi.Length];
					using (FileStream ifs = fi.OpenRead())
						ifs.Read(buffer, 0, buffer.Length);
					int bone_count = BitConverter.ToInt32(buffer, 0);

					if (bone_count * (4 + 3 * 4 + 3 * 4 + 3 * 4 + 3 * 4) + 4 != buffer.Length)
						throw new Exception(fi.Name + " file size incorrect");

					sknbuffer.Write(buffer, 4, buffer.Length - 4);
					sknbuffer.Flush();
					skns.Write(BitConverter.GetBytes(total_bone_count), 0, 4);
					skns.Write(BitConverter.GetBytes(bone_count), 0, 4);
					skns.Flush();
					total_bone_count += bone_count;
					sknTOC.Add(fi.Name.Replace('.', '_').ToUpperInvariant());
				}
			}
			hamster.Write(BitConverter.GetBytes(total_bone_count), 0, 4);
			hamster.Write(sknbuffer.ToArray(), 0, (int)sknbuffer.Length);
			hamster.Flush();
			sknbuffer.Close();

			// SKN
			hamster.Write(BitConverter.GetBytes(sknTOC.Count), 0, 4);
			hamster.Write(skns.ToArray(), 0, (int)skns.Length);
			hamster.Flush();
			skns.Close();

			MemoryStream animbuffer = new MemoryStream();
			MemoryStream anims = new MemoryStream();

			// ANIM_BUFFER
			foreach (string s in Directory.EnumerateFiles(root + "\\animations"))
			{
				FileInfo fi = new FileInfo(s);
				if (fi.Extension.Equals(".anim"))
				{
					Console.WriteLine("Writing " + fi.Name);

					byte[] buffer = new byte[fi.Length];
					using (FileStream ifs = fi.OpenRead())
						ifs.Read(buffer, 0, buffer.Length);
					int frame_count = BitConverter.ToInt32(buffer, 0);
					int bone_count = BitConverter.ToInt32(buffer, 4);

					if (frame_count * bone_count * (3 * 4 + 4 * 4 + 3 * 4) + 8 != buffer.Length)
						throw new Exception(fi.Name + " file size incorrect");

					animbuffer.Write(buffer, 8, buffer.Length - 8);
					animbuffer.Flush();
					anims.Write(BitConverter.GetBytes(total_xf_count), 0, 4);
					anims.Write(BitConverter.GetBytes(frame_count), 0, 4);
					anims.Flush();
					total_xf_count += frame_count * bone_count;
					animTOC.Add(fi.Name.Replace('.', '_').ToUpperInvariant());
				}
			}
			hamster.Write(BitConverter.GetBytes(total_xf_count), 0, 4);
			hamster.Write(animbuffer.ToArray(), 0, (int)animbuffer.Length);
			hamster.Flush();
			animbuffer.Close();

			// ANIM
			hamster.Write(BitConverter.GetBytes(animTOC.Count), 0, 4);
			hamster.Write(anims.ToArray(), 0, (int)anims.Length);
			hamster.Flush();
			anims.Close();

            MemoryStream texturebuffer = new MemoryStream();
            MemoryStream textures = new MemoryStream();

            // TEXTURE_BUFFER
            foreach (string s in Directory.EnumerateFiles(root + "\\textures"))
            {
                FileInfo fi = new FileInfo(s);
                if (fi.Extension.Equals(".png"))
                {
                    Console.WriteLine("Writing " + fi.Name);

                    byte[] buffer = new byte[fi.Length];
                    using (FileStream ifs = fi.OpenRead())
                        ifs.Read(buffer, 0, buffer.Length);

                    texturebuffer.Write(buffer, 0, buffer.Length);
                    texturebuffer.Flush();
                    textures.Write(BitConverter.GetBytes(total_png_size), 0, 4);
                    textures.Write(BitConverter.GetBytes(buffer.Length), 0, 4);
                    textures.Flush();
                    total_png_size += buffer.Length;
                    pngTOC.Add(fi.Name.Replace('.', '_').ToUpperInvariant());
                }
            }
            hamster.Write(BitConverter.GetBytes(total_png_size), 0, 4);
            hamster.Write(texturebuffer.ToArray(), 0, (int)texturebuffer.Length);
            hamster.Flush();
            texturebuffer.Close();

            // TEXTURE
            hamster.Write(BitConverter.GetBytes(pngTOC.Count), 0, 4);
            hamster.Write(textures.ToArray(), 0, (int)textures.Length);
            hamster.Flush();
            textures.Close();

            hamster.Close();

			Console.WriteLine("Writing TOC.h");

			// TOC.h
			FileStream TOC = File.Create(root + "\\TOC.h");
			string header = "#pragma once\n\nnamespace Hamster\n{\n\tnamespace TOC\n\t{\n\t\tconst unsigned int VERSION = " + BitConverter.ToUInt32(version, 0) + ";\n";
			TOC.Write(Encoding.UTF8.GetBytes(header), 0, header.Length);
			for (int i = 0; i < meshTOC.Count; i++)
			{
				string entry = "\t\tconst unsigned int " + meshTOC[i] + " = " + i + ";\n";
				TOC.Write(Encoding.UTF8.GetBytes(entry), 0, entry.Length);
			}
			for (int i = 0; i < sknTOC.Count; i++)
			{
				string entry = "\t\tconst unsigned int " + sknTOC[i] + " = " + i + ";\n";
				TOC.Write(Encoding.UTF8.GetBytes(entry), 0, entry.Length);
			}
			for (int i = 0; i < animTOC.Count; i++)
			{
				string entry = "\t\tconst unsigned int " + animTOC[i] + " = " + i + ";\n";
				TOC.Write(Encoding.UTF8.GetBytes(entry), 0, entry.Length);
			}
            for (int i = 0; i < pngTOC.Count; i++)
            {
                string entry = "\t\tconst unsigned int " + pngTOC[i] + " = " + i + ";\n";
                TOC.Write(Encoding.UTF8.GetBytes(entry), 0, entry.Length);
            }
            TOC.Write(Encoding.UTF8.GetBytes("\t}\n}"), 0, 4);
			TOC.Flush();
			TOC.Close();

			Console.WriteLine("Finished writing version " + BitConverter.ToUInt32(version, 0));
			Console.ReadLine();
		}
	}
}
