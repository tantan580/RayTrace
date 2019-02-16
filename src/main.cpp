#include <iostream>
#include "vec3.h"
#include "rays.h"
#include "hitable_list.h"
#include "sphere.h"
#include "camera.h"
#include "material.h"
#include "moving_sphere.h"
#include "bvh.h"
#include "perlin.h"

//depth：进行多少次光线追踪
vec3 color(const ray &r, hitable *world, int depth)
{
	hit_record rec;
	if (world->hit(r, 0.001, MAXFLOAT, rec))
	{
		ray scattered;//散射光线
		vec3 attenuation;//反射率
		if (depth < 20 && rec.mat_ptr->scatter(r,rec,attenuation,scattered))//调用两个派生类进行分别的渲染
			return attenuation * color(scattered, world, depth + 1);
		else
			return vec3(0,0,0);
	}
	else
	{
		vec3 unit_direction = unit_vector(r.direction());//归一化成单位坐标
		float t = 0.5 * (unit_direction.y() + 1.0f);//全部变成正数方便混色，t=1时变成blue，t=0时变成white
		return (1.0f - t) * vec3(1.0f, 1.0f, 1.0f) + t * vec3(0.5f, 0.7f, 1.0f);//就是混色操作，类似线性插值
	}
}

hitable *random_scene(){
	int n = 200;//200个球
	texture *checker = new checker_texture(new constant_texture(vec3(0.2, 0.3, 0.1)),
										 new constant_texture(vec3(0.9, 0.9, 0.9)));
	hitable **list = new hitable *[n + 1];
	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(checker));//大地表面背景
	int i = 1;
	for (int a = -5; a < 5; a++)
	{
		for (int b = -5; b < 5; b++)
		{
			float choose_mat = drand48();
			vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
			if ((center - vec3(4, 0.2, 1)).length() > 0.9)
			{
				if (choose_mat < 0.8)
				{  // diffuse
					list[i++] = new moving_sphere(center, center + vec3(0, 0.5 * drand48(), 0), 0.0, 1.0, 0.2,
												  new lambertian(new constant_texture(
														  vec3(drand48() * drand48(), drand48() * drand48(),
															   drand48() * drand48()))));
				}
				else if (choose_mat < 0.95)
				{ // metal
					list[i++] = new sphere(center, 0.2,
										   new metal(vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()),
														  0.5 * (1 + drand48())), 0.5 * drand48()));
				}
				else
				{  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constant_texture(vec3(0.4, 0.2, 0.1))));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new bvh_node(list, i, 0, 1);
}

hitable *two_perlin_spheres()
{
	texture *pertext = new noise_texture(1.0);
	hitable **list = new hitable* [2];
	list[0] = new sphere(vec3(0,-1000,0),1000,new lambertian(pertext));
	list[1] = new sphere(vec3(0,2,0),2,new lambertian(pertext));
	return new hitable_list(list,2);
}


int main(int argc, char* argv[])
{
	//画面是200*100
	int nx = 200;
	int ny = 100;
	int ns = 100;//对一个像素点重复采样进行抗锯齿

	std::cout << "P3\n" << nx << " " << ny << "\n255\n";

	//hitable *world = random_scene();
	hitable *world = two_perlin_spheres();

	vec3 lookfrom(12,2,3);
	vec3 lookat(0,0,0);
	const vec3 vup(0,1,0);
	const float fov = 20.0;
	float dist_to_focus = 10;//焦距长度 为对焦到lookat位置的 长度
	float aperture = 0.0;//光圈（透镜）大小
	camera cam(lookfrom, lookat, vup, fov, float(nx) / float(ny), aperture, dist_to_focus, 0.0, 1.0);

	for (int j = ny - 1; j >= 0; j--)
	{
		for (int i = 0; i < nx; i++)
		{
			vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++)//通过ns次的模糊化后，进行抗锯齿
			{
				//drand48生成一个[0,1)之间的double
				float u = float(i + drand48()) / float(nx);
				float v = float(j + drand48()) / float(ny);
				ray r = cam.get_ray(u, v);
				vec3 p = r.point_at_parameter(2.0);
				col += color(r, world, 0);
			}
			col /= float(ns);

			int ir = int(255.99 * col[0]);
			int ig = int(255.99 * col[1]);
			int ib = int(255.99 * col[2]);

			std::cout << ir << " " << ig << " " << ib << "\n";
		}
	}
}