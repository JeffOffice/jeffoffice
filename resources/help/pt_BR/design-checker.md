# Verificar o design
<!-- keywords: verificador de design, verificar, erros, problemas, baixa resolução, excesso, texto não cabe, caixa de texto vazia, fora da página, pré-impressão, comprovação, verificação final, prova, imagem vinculada, fonte ausente -->

O **Verificador de design** examina sua publicação em busca de problemas comuns. Ele encontra texto que não cabe, caixas vazias, objetos saindo da página e imagens que serão impressas com baixa qualidade. Execute-o antes de imprimir ou de enviar um arquivo para uma gráfica.

## Executar o Verificador de design

Clique em **Revisão > Verificar > Executar verificador de design**. O painel **Verificador de design** é aberto à direita. Você também pode clicar em **Arquivo > Propriedades** e em **Executar verificador de design**.

O painel tem duas caixas de seleção, ambas marcadas inicialmente:

- **Executar verificações gerais de design** procura problemas de layout.
- **Executar verificações finais de publicação** procura problemas que importam na impressão.

Deixe as duas marcadas para ver todos os tipos de problema. A lista é atualizada enquanto você trabalha. Se não houver nada para corrigir, ela mostra "Nenhum problema encontrado."

## Corrigir um problema

1. Clique em um item da lista em **Selecione um item para corrigir**.
2. Clique em **Ir para o Item**. Ou clique duas vezes no item.

O JeffPub vai até a página e seleciona o objeto. Corrija-o, e o item sai da lista.

## O que ele encontra

| Mensagem | O que significa | O que fazer |
|---|---|---|
| A página não tem conteúdo | A página está vazia. | Adicione algo ou exclua a página. |
| O objeto está parcialmente fora da página | Parte de um objeto está fora da página. | Mova-o ou deixe-o assim se for uma sangria. |
| A caixa de texto está vazia | Uma caixa de texto não contém texto. | Digite nela ou exclua-a. |
| Matéria com texto em excesso | Uma matéria é o texto que passa por caixas vinculadas. Esta tem mais texto do que cabe. | Aumente uma caixa, corte texto ou [vincule outra caixa](linked-text). |
| O texto foi muito reduzido para caber | O ajuste automático, que reduz o texto para caber na caixa, deixou o texto com menos da metade do tamanho. | Aumente a caixa ou use menos texto. |
| O espaço reservado para imagem está vazio | Um quadro de imagem não tem imagem. | Clique nele para adicionar uma ou exclua-o. |
| A imagem não está dimensionada proporcionalmente | A imagem está esticada ou achatada. | Redimensione-a para restaurar a forma original. |
| A imagem está vinculada, não inserida | A imagem está ligada a um arquivo e somente uma pequena visualização é armazenada na publicação. Uma imagem inserida com **Inserir e Vincular** é armazenada inteira e não aparece na lista. | Mantenha o arquivo original junto com a publicação ou clique em **Inserir Imagem** no [Gerenciador de Elementos Gráficos](graphics-manager). |
| A imagem vinculada está ausente | O arquivo para o qual a imagem vinculada aponta não existe mais, então a página mostra apenas a pequena visualização, e é isso que seria impresso. | No Gerenciador de Elementos Gráficos, clique em **Alterar Vínculo…** para localizar o arquivo, ou coloque-o de volta e clique em **Atualizar Vínculo**. |
| A imagem vinculada mostra apenas a visualização até Atualizar Vínculo | A imagem está vinculada a um arquivo fora da pasta da publicação, que o JeffPub não abre sozinho. A página mostra a pequena visualização, e é isso que seria impresso. | No Gerenciador de Elementos Gráficos, selecione a imagem e clique em **Atualizar Vínculo**. |
| A imagem tem baixa resolução | A imagem tem menos de 150 ppi (pixels por polegada) no tamanho impresso. | Use uma imagem maior ou imprima-a menor. |
| O objeto tem transparência ou efeitos (pode ser impresso de forma diferente) | Áreas transparentes e efeitos podem ficar diferentes no papel. | Imprima uma página de teste. |
| A fonte "nome" não está instalada | Uma fonte do texto não está neste computador, então outra é usada no lugar. | Instale a fonte ou escolha outra. |

Cada mensagem termina com o número da página, como "(Página 2)".

## Antes de gerar um arquivo para a gráfica

Quando você cria um arquivo PDF/X, o JeffPub avisa sobre imagens de baixa resolução e fontes ausentes e oferece abrir o Verificador de design. Consulte [Arquivos PDF e gráficas](pdf).

## Relacionados

- [Arquivos PDF e gráficas](pdf)
- [Imprimir uma publicação](printing)
- [Fazer o texto fluir entre caixas de texto](linked-text)
- [Adicionar e ajustar imagens](pictures)
- [Gerenciador de Elementos Gráficos](graphics-manager)
