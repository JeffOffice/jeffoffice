# Trabalhar com ficheiros .pub

<!-- keywords: pub, .pub, ficheiro pub, abrir pub, guardar pub, converter, compatibilidade, importar, publicações antigas, outros programas, jpub, formato, tipos de letra em falta, cópia de segurança do original, formato de ficheiro -->

Um ficheiro .pub é uma publicação criada noutros programas de paginação eletrónica. O JeffPub abre ficheiros .pub criados desde 1998. Também pode guardar o seu trabalho como ficheiro .pub. O formato próprio do JeffPub é o .jpub.

## Abrir um ficheiro .pub

1. Prima Ctrl+O, ou clique em **Ficheiro > Abrir**.
2. Clique em **Procurar**, escolha o ficheiro .pub e clique em **Abrir**.

Também pode arrastar o ficheiro para a página. O texto, as imagens, as formas e as tabelas são importados, juntamente com as caixas de texto ligadas. A maioria das páginas fica muito semelhante ao original, mas nem todos os pormenores coincidem ainda. Algumas caixas de texto podem terminar uma ou duas palavras antes ou depois do que terminavam antes.

Se um tipo de letra do ficheiro não estiver no seu computador, o JeffPub utiliza um tipo de letra livre semelhante. Os tipos de letra incorporados no ficheiro .pub não são importados. Se o JeffPub tiver algo a comunicar depois de abrir o ficheiro, aparece uma nota na barra de estado, na parte inferior da janela.

Se o ficheiro não abrir, o JeffPub indica que pode estar danificado ou que utiliza funcionalidades que ainda não consegue ler. O mesmo acontece com um ficheiro que contém texto ou formas, mas nenhum que o JeffPub consiga colocar numa página: não é aberto como páginas em branco, para que não o substitua por engano, e o ficheiro fica como estava.

## Verificar o resultado

Abrir um ficheiro é uma conversão, por isso verifique todas as páginas. Confirme as quebras de linha, os tipos de letra e as imagens. Utilize o [Verificador de Estrutura](design-checker) para encontrar texto que não caiba.

## Guardar como ficheiro .pub

- Clique em **Ficheiro > Exportar** e, a seguir, em **Guardar como Ficheiro .pub**.
- Ou clique em **Ficheiro > Guardar Como** e escolha **Ficheiros de Publicação .pub (*.pub)** em **Guardar como tipo**.

Quando o JeffPub guarda um ficheiro .pub, pode gravar páginas, caixas de texto e caixas de texto ligadas, formatação de texto, formas, linhas, imagens, tabelas, preenchimentos, sombras, Texto Artístico, estilos, folhetos, cartões dobrados, envelopes, números de página e ligações. Se algo não tiver forma .pub, o JeffPub indica quantos objetos deixou de fora. Esses objetos continuam na sua publicação e são mantidos quando guardar como ficheiro .jpub. Os comentários também não têm forma .pub: o JeffPub indica que os deixou de fora, e só são mantidos em ficheiros .jpub. O tópico Comentários desta Ajuda dá mais informações.

## Proteger o original

Quando abre um ficheiro .pub e prime Ctrl+S, o JeffPub grava as alterações nesse mesmo ficheiro .pub. Para manter o original seguro:

- Ative **Criar sempre cópia de segurança** em **Ficheiro > Opções**, no separador **Guardar**. O JeffPub mantém então o ficheiro anterior com o prefixo "Cópia de segurança de" seguido do nome.
- Ou comece por **Ficheiro > Guardar Como** e guarde um ficheiro .jpub. Um ficheiro .jpub mantém tudo.

Guarde os ficheiros originais até ter a certeza de que os novos estão corretos.

## Relacionados

- [Abrir e guardar publicações](open-save)
- [Guardar como imagem, página Web ou livro eletrónico](export)
- [Alterar as definições do JeffPub](options)
