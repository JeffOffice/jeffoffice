# Verificar seu design

<!-- keywords: verificador de design, verificar, erros, problemas, baixa resolução, excesso, texto não cabe, caixa de texto vazia, fora da página, verificação final, prova, imagem vinculada, fonte ausente, pré-impressão, checagem -->

O **Verificador de Design** examina sua publicação em busca de problemas que costumam acontecer. Ele encontra textos que não cabem, caixas vazias, objetos que saem da página e imagens que vão imprimir mal. Execute-o antes de imprimir ou de enviar um arquivo para a gráfica.

## Executar o Verificador de Design

Clique em **Revisão > Verificar > Executar Verificador de Design**. O painel **Verificador de Design** abre à direita. Você também pode clicar em **Arquivo > Informações** e depois em **Executar Verificador de Design**.

O painel tem duas caixas de seleção, ambas marcadas no início:

- **Executar verificações gerais de design** procura problemas de layout.
- **Executar verificações finais de publicação** procura itens importantes na impressão.

Deixe as duas marcadas para ver todos os tipos de problema. A lista é atualizada conforme você trabalha. Se não houver nada a corrigir, aparece a mensagem "Nenhum problema encontrado."

## Corrigir um problema

1. Clique em um item da lista em **Selecione um item para corrigir**.
2. Clique em **Ir para o Item**. Ou clique duas vezes no item.

O JeffPub vai até a página e seleciona o objeto. Corrija o problema e o item sai da lista.

## O que ele encontra

| Mensagem | O que significa | O que fazer |
|---|---|---|
| A página não tem conteúdo | A página está vazia. | Adicione algo ou exclua a página. |
| O objeto está parcialmente fora da página | Parte de um objeto está fora da página. | Mova-o ou deixe-o se for uma sangria. |
| A caixa de texto está vazia | Uma caixa de texto não contém texto. | Digite nela ou exclua-a. |
| Matéria com texto em excesso | Uma matéria é o texto que passa por caixas vinculadas. Esta tem mais texto do que cabe. | Aumente uma caixa, corte texto ou [vincule outra caixa](linked-text). |
| O texto foi muito reduzido para caber | Reduzir o texto para caber na caixa deixou-o com menos da metade do tamanho original. | Aumente a caixa ou use menos texto. |
| O espaço reservado para imagem está vazio | Um quadro de imagem não tem imagem. | Clique nele para adicionar uma imagem ou exclua-o. |
| A imagem não está dimensionada proporcionalmente | A imagem está esticada ou comprimida. | Redimensione-a para restaurar sua proporção. |
| A imagem está vinculada, não inserida | A imagem está ligada a um arquivo, e apenas uma pequena prévia fica armazenada na publicação. Uma imagem inserida com **Inserir e Vincular** é armazenada por inteiro e não aparece na lista. | Mantenha o arquivo original junto com sua publicação ou clique em **Inserir Imagem** no [Gerenciador de Elementos Gráficos](graphics-manager). |
| A imagem vinculada está ausente | O arquivo ao qual a imagem vinculada aponta não existe mais, então a página mostra apenas a pequena prévia, e é isso que seria impresso. | No Gerenciador de Elementos Gráficos, clique em **Alterar Vínculo…** para localizar o arquivo, ou recoloque-o e clique em **Atualizar Vínculo**. |
| A imagem vinculada mostra só a prévia até Atualizar Vínculo | A imagem está vinculada a um arquivo fora da pasta da publicação, que o JeffPub não abre automaticamente. A página mostra a pequena prévia, e é isso que seria impresso. | No Gerenciador de Elementos Gráficos, selecione a imagem e clique em **Atualizar Vínculo**. |
| A imagem tem baixa resolução | A imagem tem menos de 150 ppi (pixels por polegada) no tamanho impresso. | Use uma imagem maior ou imprima-a menor. |
| O objeto tem transparência ou efeitos (pode ser impresso de forma diferente) | Áreas transparentes e efeitos podem ficar diferentes no papel. | Imprima uma página de teste. |
| A fonte "nome" não está instalada | Uma fonte do seu texto não está neste computador, então outra é usada no lugar. | Instale a fonte ou escolha outra. |

Cada mensagem termina com o número da página, como "(Página 2)".

## Antes de um arquivo para a gráfica

Ao criar um arquivo PDF/X, o JeffPub avisa sobre imagens de baixa resolução e fontes ausentes e oferece abrir o Verificador de Design. Veja [Arquivos PDF e gráficas](pdf).

## Consulte também

- [Arquivos PDF e gráficas](pdf)
- [Imprimir uma publicação](printing)
- [Fluir texto entre caixas de texto](linked-text)
- [Adicionar e ajustar imagens](pictures)
- [Gerenciador de Elementos Gráficos](graphics-manager)
